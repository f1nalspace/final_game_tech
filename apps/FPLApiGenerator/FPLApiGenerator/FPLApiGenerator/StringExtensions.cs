namespace FPLApiGenerator
{
    static class StringExtensions
    {
        /// <summary>
        /// Converts the specified <paramref name="str"/> to pascal case, that starts the first letter as upper case.
        /// </summary>
        /// <param name="str">The <see cref="string"/>.</param>
        /// <returns>The resulting <see cref="string"/>.</returns>
        public static string ToPascalCase(this string str)
        {
            if (str is null || str.Length == 0)
                return str;
            if (str.Length == 1)
                return str.ToUpper();
            else
                return $"{char.ToUpper(str[0])}{str.Substring(1)}";
        }

        /// <summary>
        /// Converts the specified <paramref name="str"/> to camel case, that starts the first letter as lower case.
        /// </summary>
        /// <param name="str">The <see cref="string"/>.</param>
        /// <returns>The resulting <see cref="string"/>.</returns>
        public static string ToCamelCase(this string str)
        {
            if (str is null || str.Length == 0)
                return str;
            if (str.Length == 1)
                return str.ToLower();
            else
                return $"{char.ToLower(str[0])}{str.Substring(1)}";
        }
    }
}
