#pragma once

namespace Halloween::ghostSting {
    // Begin exactly once for each qualifying results screen; async load may
    // delay start slightly on the first use. No looping.
    void EnterResults();
    // Stop immediately on Continue / Restart / scene exit.
    void ExitResults();
}
