using System;
using System.Text;

namespace FPLApiGenerator
{
    class EightCC
    {
        private readonly byte[] data = new byte[8];

        public EightCC(ulong u64)
        {
            data[0] = (byte)(u64 >> 0 & 0xFF);
            data[1] = (byte)(u64 >> 8 & 0xFF);
            data[2] = (byte)(u64 >> 16 & 0xFF);
            data[3] = (byte)(u64 >> 24 & 0xFF);
            data[4] = (byte)(u64 >> 32 & 0xFF);
            data[5] = (byte)(u64 >> 40 & 0xFF);
            data[6] = (byte)(u64 >> 48 & 0xFF);
            data[7] = (byte)(u64 >> 56 & 0xFF);
        }

        public EightCC(ReadOnlySpan<char> str)
        {
            Array.Clear(data, 0, data.Length);
            for (int i = 0; i < Math.Min(8, str.Length); i++)
                data[i] = Convert.ToByte(str[i]);
            
        }

        public EightCC(byte[] ba)
        {
            Array.Clear(data, 0, data.Length);
            for (int i = 0; i < Math.Min(8, ba.Length); i++)
                data[i] = ba[i];
        }

        public ulong ToU64()
        {
            ulong result = 0;
            result |= (ulong)data[0] << 0;
            result |= (ulong)data[1] << 8;
            result |= (ulong)data[2] << 16;
            result |= (ulong)data[3] << 24;
            result |= (ulong)data[4] << 32;
            result |= (ulong)data[5] << 40;
            result |= (ulong)data[6] << 48;
            result |= (ulong)data[7] << 56;
            return result;
        }

        public string ToHex() => ToU64().ToString("X16");

        public override string ToString() => Encoding.UTF8.GetString(data, 0, 8);
    }
}
