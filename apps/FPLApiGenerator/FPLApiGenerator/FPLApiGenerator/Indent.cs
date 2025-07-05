using System;
using System.Diagnostics;

namespace FPLApiGenerator
{
    public class IndentState
    {
        public int Level { get; private set; } = 0;

        internal int Inc()
        {
            return ++Level;
        }

        internal void Dec()
        {
            Debug.Assert(Level > 0);
            --Level;
        }
    }

    public struct Indent : IDisposable
    {
        public IndentState State { get; }
        public string Text { get; }

        private bool _disposed = false;

        public Indent(IndentState state)
        {
            int level = state.Inc();
            State = state ?? throw new ArgumentNullException(nameof(state));
            Text = level > 0 ? new string('\t', level) : string.Empty;
        }

        public override readonly string ToString() => Text;

        public void Dispose()
        {
            if (_disposed)
                return;
            _disposed = true;
            State.Dec();
        }
    }
}
