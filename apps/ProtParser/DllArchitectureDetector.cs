using System;
using System.IO;

namespace ProtParser
{
    public enum DllArchitecture
    {
        Unknown = 0,
        X86 = 1,
        AMD64 = 2
    }

    public static class DllArchitectureDetector
    {
        /// <summary>
        /// Returns the architecture of a PE file (DLL, EXE, etc.).
        /// </summary>
        public static DllArchitecture GetArchitecture(string path)
        {
            if (!File.Exists(path))
                throw new FileNotFoundException("The specified file does not exist.", path);

            try
            {
                using var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
                using var br = new BinaryReader(fs);

                // 1. Get the offset to the PE header (e_lfanew) – stored at 0x3C.
                const int e_lfanewOffset = 0x3C;
                fs.Seek(e_lfanewOffset, SeekOrigin.Begin);
                int peHeaderOffset = br.ReadInt32();          // 4‑byte little endian

                // 2. Go to the PE header and validate the "PE\0\0" signature.
                fs.Seek(peHeaderOffset, SeekOrigin.Begin);
                uint signature = br.ReadUInt32();
                if (signature != 0x00004550)                 // "PE\0\0"
                    return DllArchitecture.Unknown;

                // 3. IMAGE_FILE_HEADER – we only need the first field (Machine).
                ushort machine = br.ReadUInt16();             // Machine
                                                              // Skip the rest of IMAGE_FILE_HEADER (20 bytes total, we've read 2 already)
                fs.Seek(18, SeekOrigin.Current);

                // 4. Optional header magic tells us PE32 vs PE32+.
                ushort optionalHeaderMagic = br.ReadUInt16();

                return optionalHeaderMagic switch
                {
                    0x10b => DllArchitecture.X86,    // PE32 – 32‑bit
                    0x20b => DllArchitecture.AMD64,  // PE32+ – 64‑bit
                    _ => DllArchitecture.Unknown
                };
            }
            catch (Exception)
            {
                // Any read error or corruption → Unknown.
                return DllArchitecture.Unknown;
            }
        }

        /// <summary>
        /// Convenience helper that returns true if the file is a 64‑bit DLL/PE.
        /// </summary>
        public static bool Is64Bit(string path) =>
            GetArchitecture(path) == DllArchitecture.AMD64;

        /// <summary>
        /// Convenience helper that returns true if the file is a 32‑bit DLL/PE.
        /// </summary>
        public static bool Is32Bit(string path) =>
            GetArchitecture(path) == DllArchitecture.X86;
    }

}
