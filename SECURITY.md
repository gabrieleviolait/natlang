# Security policy

## Support status

NatLang **v0.1 is an experimental compiler** and is not yet hardened to compile hostile input in an untrusted environment. No formally maintained security support window exists for this prototype.

## Important boundaries

- `.nat` programs may read and overwrite files through their compiled executables.
- The executable is unsandboxed. Do not run generated applications with privileges they don't need.
- The compiler shells out to a system C++ toolchain and (optionally) `curl`; treat executable paths, file paths, toolchain configuration and `.nat` input as trusted until child-process spawning is hardened.
- The local model adapter accepts loopback URLs only, but model output can still misinterpret the intended behavior.
- Generated programs have runtime checks for some dynamic errors, not memory/resource or time guarantees.

## Reporting

Please report potential vulnerabilities privately to the project owner via GitHub's **Report a vulnerability** feature if enabled. Avoid publishing exploit details in a public issue before there is a mitigation. General correctness bugs may be reported in issues.
