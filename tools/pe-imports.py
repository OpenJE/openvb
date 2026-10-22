#!/usr/bin/env python3
"""tools/pe-imports.py — PE import/export/version reader for F3-era binaries.
Graduated from SwiftMoon's /tmp/peinfo.py (R-CODEC session) WITH fix #3 applied:
the saved /tmp copy still dereferenced OriginalFirstThunk(+0) as the Name RVA —
descriptor layout is (OrigFT@0, TimeDateStamp@4, FwdChain@8, Name@12, FirstThunk@16);
correct thunk source is OrigFT, falling back to FirstThunk. Selftest compares against
objdump-derived ground truth for F3.exe. Usage: python3 tools/pe-imports.py <pe> | --selftest
"""
import struct, subprocess, sys
from pathlib import Path

F3 = "/nix/store/yhdpc35v4hmgn9b5hkfmis1wzvwpprf0-f3demo-1.0/share/f3demo/F3.exe"

def pe(path):
    d = Path(path).read_bytes()
    nt = struct.unpack('<I', d[0x3c:0x40])[0]
    nsecs = struct.unpack('<H', d[nt+6:nt+8])[0]
    sizeopt = struct.unpack('<H', d[nt+20:nt+22])[0]
    opt = nt + 24
    magic = struct.unpack('<H', d[opt:opt+2])[0]
    dd = opt + (96 if magic == 0x10b else 112)
    ed_rva = struct.unpack('<I', d[dd+0:dd+4])[0]
    iid_rva = struct.unpack('<I', d[dd+8:dd+12])[0]
    if nsecs > 96 or d[:2] != b'MZ':
        raise ValueError("not a sane PE (refusing to allocate against garbage headers)")
    shdr = opt + sizeopt
    secs = []
    for i in range(nsecs):
        o = shdr + i*40
        name = d[o:o+8].rstrip(b'\0').decode(errors='replace')
        vs, va, raw, ro = struct.unpack('<IIII', d[o+8:o+24])
        if ro:
            secs.append((va, max(vs, raw), ro))
    def off(rva):
        for va, span, ro in secs:
            if va <= rva < va + span:
                return ro + (rva - va)
        return None
    def cstr(o):
        e = d.index(b'\0', o); return d[o:e].decode(errors='replace')

    exp = {}
    if ed_rva and off(ed_rva):
        eo = off(ed_rva)
        nfuncs, nnames = struct.unpack('<II', d[eo+20:eo+28])
        af, an, ao = (off(x) for x in struct.unpack('<III', d[eo+28:eo+40]))
        if None not in (af, an, ao):
            for i in range(nnames):
                nm = cstr(off(struct.unpack('<I', d[an+i*4:an+i*4+4])[0]))
                fidx = struct.unpack('<H', d[ao+i*2:ao+i*2+2])[0]
                exp[nm] = struct.unpack('<I', d[af+fidx*4:af+fidx*4+4])[0]

    imps = []
    o = off(iid_rva) if iid_rva else None
    while o is not None:
        origft, tsz, fwd, nam, ft = struct.unpack('<IIIII', d[o:o+20])
        if nam == 0 and origft == 0 and ft == 0:
            break
        dll = cstr(off(nam))
        th = off(origft) or off(ft)
        funcs = []
        if th:
            j = 0
            while True:
                t = struct.unpack('<I', d[th+j*4:th+j*4+4])[0]; j += 1
                if t == 0: break
                if t & 0x80000000: funcs.append(f"#{t & 0xffff}")
                elif off(t): funcs.append(cstr(off(t)+2))
        imps.append((dll, funcs))
        o += 20

    ver = {}
    for tag in (b'F\x00i\x00l\x00e\x00V\x00e\x00r\x00s\x00i\x00o\x00n\x00',
                b'P\x00r\x00o\x00d\x00u\x00c\x00t\x00V\x00e\x00r\x00s\x00i\x00o\x00n\x00'):
        i = d.find(tag)
        if i > 0:
            k = i + len(tag)
            while k < len(d)-1 and d[k] in (0,1,2,3,4,8): k += 1
            end = d.find(b'\x00\x00', k)
            if 0 < end-k <= 64:
                try: ver[tag.decode('utf-16-le')] = d[k:end].decode('utf-16-le')
                except Exception: pass
    return secs, exp, imps, ver

def main():
    if "--selftest" in sys.argv:
        _, _, imps, _ = pe(F3)
        mine = sorted(dll.lower() for dll, _ in imps)
        truth = sorted(l.split()[-1].lower() for l in subprocess.run(
            ['objdump','-p',F3], capture_output=True, text=True).stdout.splitlines()
            if 'DLL Name' in l)
        assert mine == truth, f"SELFTEST FAIL:\n mine={mine}\n objd={truth}"
        for dll, funcs in imps:
            if dll.lower() == 'granny2.dll':
                named = [f for f in funcs if f.startswith('_Granny')]
                assert len(named) == 43, f"granny2 NAMED-import count {len(named)} != 43"
                assert not [f for f in funcs if f.startswith('#')], "granny2 has no ordinal-only imports (the '44 ordinals' claim is a dead awk column)"
        print(f"selftest PASS: {len(mine)} DLLs match objdump; granny2 = 43 NAMED imports, zero ordinals")
        return
    _, exp, imps, ver = pe(sys.argv[1])
    for dll, funcs in imps:
        print(f"{dll}: {len(funcs)} imports" + (f" [{funcs[0]}...]" if funcs else ""))
    if exp: print(f"exports: {len(exp)}")
    if ver: print(f"version: {ver}")

if __name__ == '__main__':
    main()
