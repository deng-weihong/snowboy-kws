# Third-party components

| Component | Version / target | License | SHA-256 |
| --- | --- | --- | --- |
| Snowboy engine | 1.3.0, ARMv7 hard-float | Apache-2.0 | `346db1193490a9cc404d49fcfb22ca612cd3a0e649c4863f411553eb1c4f9f1f` |
| Snowboy `common.res` | 1.3.0 | Apache-2.0 | `5dd5258678182f2e055fa7a6167eba50ded3bf8b41f70faab11fd9b221de488b` |
| OpenBLAS | 0.3.21, ARMv7 | BSD-3-Clause | `147ea334ef86ce4234fb5ce8a6fb25d542e2c2c41f6f7a89ffe56c18a7b427ab` |

Snowboy was discontinued by KITT.AI in 2020. Its engine is supplied as an
upstream precompiled static archive; the detection engine source is not part of
this repository. The bundled archive has been verified on RV1106 with uClibc,
ARMv7-A, hard-float, NEON, and VFPv4.

The bundled OpenBLAS archive was built for the same ARMv7 hard-float ABI.
Applications must use a compatible C++ runtime and C library ABI.
