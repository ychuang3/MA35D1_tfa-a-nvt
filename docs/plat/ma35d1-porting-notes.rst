MA35D1 migration findings and board-test follow-up
=================================================

This log tracks findings while porting the Nuvoton TF-A v2.3 BSP to
TF-A v2.15.0. Board tests are intentionally deferred until all BSP patches
have been ported. A successful build is not evidence of hardware operation.
Keep issue IDs stable and retain findings after a fix, recording the source
commit, resolution and validation evidence. Later commits may supersede an
intermediate limitation; re-evaluate it against the final port before testing.

MA35-001: FIP backend size
-------------------------

Status: open; discovered during BSP commit ``92cdeb6ac`` (port ``fa140f8``).

The initial ``MA35D1_FIP_SIZE`` is 1 MiB. The local newer U-Boot and OP-TEE
inputs produced a 1,452,817-byte release FIP and an approximately 1.39 MiB
TBBR FIP during packaging checks. FIP creation does not enforce the backend
size, while ``io_block`` bounds access to its configured region. These test
packages therefore exceed what this intermediate port can load.

Final follow-up: compare the final FIP size, backend length, DDR buffer,
flash offsets and partition capacity. Resolve mismatches before installation,
then validate complete BL31/BL32/BL33 loading on every supported boot medium.

MA35-002: BL31 SRAM budget and Boot ROM reservation
-------------------------------------------------

Status: build mitigation verified; hardware ownership and resume pending.
Discovered during ``92cdeb6ac`` / ``fa140f8``.

The source defined a BL31 limit of ``0x28039000`` despite its adjacent
comment reserving memory from ``0x28038000`` for the Boot ROM. The port uses
``0x28038000`` as the limit. TRM Rev 1.11 Table 6.4-1 confirms the physical
SRAM1 range, but does not by itself confirm this firmware reservation.
Several validated BL31 configurations end exactly at the chosen limit.
OP-TEE debug needs ``ENABLE_LTO=1``; without it, linking exceeds the budget.
Forced debugfs was removed; opting into it may also exceed the budget.

Final follow-up: inspect the final ELF, stacks and translation tables,
confirm the Boot ROM/warm-boot memory contract, and check the reserved SRAM
through cold boot, secondary CPU startup and suspend/resume. Recheck stack
requirements after all later BSP changes have been applied.

MA35-003: OP-TEE handoff and pageable images
------------------------------------------

Status: header handoff corrected in ``fa140f8``; board validation pending.

The initial BSP overwrote the architecture and pageable arguments produced
by ``parse_optee_header``. The port preserves them for the upstream OPTEED
protocol. Release, debug with LTO and legacy ``NEED_BL32=yes`` builds pass.
The packaging checks use a header and pager image; pageable loading remains
unvalidated, and the initial IO policy has no ``BL32_EXTRA2_IMAGE_ID`` entry.

Final follow-up: verify the final IO policy and FIP entries for the chosen
OP-TEE layout. Confirm entry architecture, pager/pageable addresses, initial
arguments and secure/non-secure transitions on the board. Do not infer
pageable-image support from the header/pager packaging test.

MA35-004: Trusted Boot provisioning and version counters
------------------------------------------------------

Status: software TBBR build/packaging verified; final trust chain pending.
Discovered during ``92cdeb6ac`` / ``fa140f8``.

The initial port generates a development key, embeds its root public-key
hash in BL2, returns zero for NV counters and does not persist updates.
Mbed TLS was updated to the dependency pinned by TF-A v2.15.0. Its upstream
redundant-declaration warnings remain non-fatal under TF-A's existing rules.
This intermediate path does not establish an OTP root of trust or implement
anti-rollback. Later BSP security commits have not yet been ported.

Final follow-up: inspect the complete Boot ROM/BL2/FIP trust chain, key
provisioning and final counter implementation after those commits are
ported. Test valid and invalid signatures and the final rollback/boot-bank
behaviour using the agreed board-test procedure.

MA35-005: Intermediate BSP hardware coverage
------------------------------------------

Status: pending all later BSP commits and final board tests.
Discovered during ``92cdeb6ac`` / ``fa140f8``.

The first BSP commit contains early DDR tables and peripheral programming;
later production-silicon fixes are intentionally ported separately. The
inherited FVP topology defaults were corrected to one two-core Cortex-A35
cluster, but boot and power-management behaviour has not been measured.

Final follow-up: select the final board/DDR configuration and test the
supported storage boot paths, UART, secondary CPU startup, reset and
suspend/resume. Track any specific failure here with reproduction details.

MA35-006: Inherited coding-style findings
----------------------------------------

Status: recorded; software review follow-up, not a hardware-test item.
Discovered during ``92cdeb6ac`` / ``fa140f8``.

The imported BSP has existing checkpatch errors and warnings. The migration
adaptations passed a separate checkpatch check, and whitespace checks pass;
this does not mean the entire imported BSP passes upstream coding style.
Revisit style separately if an upstream-quality submission is required.

MA35-007: Default RTP clock enable and eventual clock ownership
-------------------------------------------------------------

Status: register definition checked; board behaviour pending.
Source BSP commit: ``6d44d4f3f9ca1362c055483a303fe4e242553b4c``.

The patch sets ``CLK_SYSCLK0[1]`` during ``plat_ma35d1_init`` using a
read/modify/write. TRM Rev 1.11, CLK_SYSCLK0 description (page 318), defines
bit 1 as the write-protected ``RTPEN`` Cortex-M4 AHB clock enable. The
existing setup unlocks ``SYS_RLKTZS`` before this write. The inherited
DDR initialisation writes ``0x35`` to the same register, clearing that bit,
so BL31 re-enables it. The source commit describes this as temporary until
an SMC service can enable/disable the RTP clock. No such later service is
introduced by this commit. Release and OP-TEE debug with LTO builds of
BL2, BL31 and both DTBs pass; no board behaviour was tested.

Final follow-up: trace later BSP clock/SMC changes to determine the final
owner and whether the unconditional enable remains necessary. Read back
RTPEN after BL31 startup, exercise the final RTP/M4 boot flow and clock
controls, and verify the expected clock state through suspend/resume.

MA35-008: KeyStore initialization syntax, completion mask and ownership
---------------------------------------------------------------------

Status: syntax adapted for compilation; known completion-mask fix pending
its own source commit; hardware behaviour untested.
Source BSP commit: ``4f4669331cf85682bb90a7d7da32e8af4b1dbbd3``.

The source adds ``KS_BASE = 0x40340000``, writes ``0x101`` to KS_CTL and
polls KS_STS for INITDONE and BUSY. This remains inside the original
``(SYS_CHIPCFG & 0x100) == 0`` branch after the crypto/KeyStore clocks are
enabled. It does not change the TBBR verification or provisioning flow.

Both source ``inp32(void *)(...)`` expressions are invalid C. They are
corrected to ``inp32((void *)(...))`` here so this port remains buildable.
The next BSP commit ``cd0420a55310bcd891d4221514ddbcaa03f46dcc`` consists
of those same syntax corrections. When processing that commit, record
that its code effect was already needed here rather than skipping its
provenance or introducing a duplicate runtime change.

This step intentionally preserves the source INITDONE mask ``0x100``.
The later BSP commit ``41c36c0329e8214b7b8f8031c90fff16ba6000e0`` corrects
it to ``0x80`` (KS_STS bit 7); that semantic fix has not been ported yet.
Consequently this intermediate version can hang in the INITDONE wait and
is not ready to boot on the board. Both polling loops lack a timeout or
error-status path. The source commit also states that OP-TEE initializes
KeyStore, leaving initialization ownership to confirm in the final stack.
TRM Rev 1.11 section 6.38 (page 2203) provides an overview and feature list,
not the KS_CTL/KS_STS register definitions. The completion-bit correction
above is evidenced by the later BSP fix, not independently by this TRM.

Validation for this step: release and OP-TEE debug with LTO builds of BL2,
BL31 and both DTBs pass. No hardware initialization was executed.

Final follow-up: reconcile the next two source commits, trace all later
KeyStore/TSI changes, confirm the final register address and bit definitions,
clock/access permissions and completion sequence on each supported chip.
Check normal and failed initialization, decide how a stuck engine should
be handled, and verify that TF-A and OP-TEE initialization cooperate with
the intended key lifetime. Run these checks after all BSP patches are ported.
