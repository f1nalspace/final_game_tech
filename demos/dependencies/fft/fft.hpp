#ifndef FFT_SINGLEHEADER_INCLUDED
#define FFT_SINGLEHEADER_INCLUDED

/* fft.hpp =========================
	Public-domain single-header library
	implementing radix-2 decimation-in-time FFT (i.e. FFT for powers of 2)

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
*/


/*
fft_core
(in_real[], in_imag[], size, gap, out_real[], out_imag[], forwards)
	in_real:    pointer to real-valued spatial samples (for audio, this is where your entire audio signal goes)
	in_imag:    pointer to imaginary-valued ones (not useful for audio)
		in_imag is allowed to be nullptr. If so, it will be treated as if it were all zeroes.
	size:       number of complex samples per domain. for audio, this is the number of real samples you have. must be a power of 2. Algorithm will definitely fail and possibly crash otherwise, not tested.
	gap:        must be 1 for outside callers. used for recursion.
	out_real:   pointer to space for real-valued output. does not need to be initialized. must be allocated.
	out_imag:   same as above, for imaginary. not optional.
		out_real and out_imag work together to store a complex number (2d vector) representing the phase and amplitude of the given frequency band, even for wholly real inputs.
	forwards:   if true, transform is forwards (fft). if false, transform is backwards (ifft).
fft
(<same as fft_core, sans [gap] and [forwards]>)
	compute forwards fft.
ifft
(<same as fft_core, sans [gap] and [forwards]>)
	compute backwards fft (inverse fft, ifft)
normalize_fft
(in_real[], in_imag[], size)
	divide the amplitude of each bin by the number of bins. obligatory after fft() for audio. modifies the input.
sanitize_fft
(in_real[], in_imag[], size)
	moves all data to positive-frequency bins. yes, FFTs have negative frequencies for some reason. they're used to retain correlation data for complex inputs. for real inputs, the negative frequencies just mirror the positive ones and sap half their amplitude, therefore this function. for an explanation of what negative frequencies mean, see http://dsp.stackexchange.com/questions/431/what-is-the-physical-significance-of-negative-frequencies .
unsanitize_fft
(in_real[], in_imag[], size)
	undo the above. note again that these two functions are nonsensical for complex inputs.
*/

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // ifndef M_PI

// address of cell if base address not nullptr, nullptr otherwise
#define fft_private_safe_addrof(ptr,i) ((ptr!=NULL)?(&(ptr[i])):(NULL))

// For a 8-sample input, the FFT's last three bins contain "negative" frequencies. (So, the last (size/2)-1 bins.) They are only meaningful for complex inputs.
void fft_core(double* input_real, double* input_imag, uint64_t size, uint64_t gap, double* output_real, double* output_imag, bool forwards)
{
	if (size == 1)
	{
		output_real[0] = input_real[0];
		if (input_imag != NULL)
			output_imag[0] = input_imag[0];
		else
			output_imag[0] = 0;
	}
	else
	{
		// This algorithm works by extending the concept of how two-bin DFTs (discrete fourier transform) work, in order to correlate decimated DFTs, recursively.
		// No, I'm not your guy if you want a proof of why it works, but it does.
		fft_core(input_real, input_imag, size / 2, gap * 2, output_real, output_imag, forwards);
		fft_core(&(input_real[gap]), fft_private_safe_addrof(input_imag, gap), size / 2, gap * 2, &(output_real[size / 2]), &(output_imag[size / 2]), forwards);
		// non-combed decimated output to non-combed correlated output
		for (uint64_t i = 0; i < size / 2; i++)
		{
			double a_real = output_real[i];
			double a_imag = output_imag[i];
			double b_real = output_real[i + size / 2];
			double b_imag = output_imag[i + size / 2];

			double twiddle_real = cos(2 * M_PI * i / size);
			double twiddle_imag = sin(2 * M_PI * i / size) * (forwards ? -1 : 1);
			// complex multiplication (vector angle summing and length multiplication)
			double bias_real = b_real * twiddle_real - b_imag * twiddle_imag;
			double bias_imag = b_imag * twiddle_real + b_real * twiddle_imag;
			// real output (sum of real parts)
			output_real[i] = a_real + bias_real;
			output_real[i + size / 2] = a_real - bias_real;
			// imag output (sum of imaginary parts)
			output_imag[i] = a_imag + bias_imag;
			output_imag[i + size / 2] = a_imag - bias_imag;
		}
	}
}

#undef fft_private_safe_addrof

#ifndef FFT_CORE_ONLY

void normalize_fft(double* input_real, double* input_imag, uint64_t size)
{
	for (uint64_t i = 0; i < size; i++)
	{
		input_real[i] /= size;
		input_imag[i] /= size;
	}
}
void half_normalize_fft(double* input_real, double* input_imag, uint64_t size)
{
	for (uint64_t i = 0; i < size; i++)
	{
		input_real[i] /= sqrt((double)size);
		input_imag[i] /= sqrt((double)size);
	}
}
void fft(double* input_real, double* input_imag, uint64_t size, double* output_real, double* output_imag)
{
	fft_core(input_real, input_imag, size, 1, output_real, output_imag, 1);
	half_normalize_fft(output_real, output_imag, size); // allows calling fft() four times to result in the original signal with no amplitude change
}
void ifft(double* input_real, double* input_imag, uint64_t size, double* output_real, double* output_imag)
{
	fft_core(input_real, input_imag, size, 1, output_real, output_imag, 0);
	half_normalize_fft(output_real, output_imag, size); // see above, also causes ifft(fft(x)) to result in the original signal with no amplitude change
}

// boost bins that are split into positive (A-handed spin) and negative (B-handed spin) parts
// only useful if former input signal was not complex, for only needing to look at one bin to get the magnitude
// FIXME or HELPME: How come the nyquist frequency is quiet in saw waves, but loud in pure signal?
void sanitize_fft(double* input_real, double* input_imag, uint64_t size)
{
	for (uint64_t i = 1; i < size / 2; i++)
	{
		input_real[i] *= 2;
		input_imag[i] *= 2;
		input_real[size - i] *= 2;
		input_imag[size - i] *= 2;
	}
}
// opposite of above
void unsanitize_fft(double* input_real, double* input_imag, uint64_t size)
{
	for (uint64_t i = 1; i < size / 2; i++)
	{
		input_real[i] /= 2;
		input_imag[i] /= 2;
		input_real[size - i] /= 2;
		input_imag[size - i] /= 2;
	}
}

#endif // ifndef FFT_CORE_ONLY

#endif // FFT_SINGLEHEADER_INCLUDED
