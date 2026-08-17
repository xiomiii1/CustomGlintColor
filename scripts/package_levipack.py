import argparse, json, re, sys, zipfile
from pathlib import Path
PAT = re.compile(r'inline\s+constexpr\s+std::string_view\s+(Name|Author|Description|Version)\s*=\s*"((?:\\.|[^"\\])*)";')
def parse_version(path):
    vals = {}
    for line in path.read_text(encoding='utf-8').splitlines():
        m = PAT.search(line)
        if m: vals[m.group(1)] = bytes(m.group(2), 'utf-8').decode('unicode_escape')
    missing = [x for x in ('Name','Author','Description','Version') if not vals.get(x)]
    if missing: raise ValueError('Missing version metadata: ' + ', '.join(missing))
    return vals
def write_package(library, icon, version_header, output):
    if not library.is_file(): raise FileNotFoundError(library)
    if not icon.is_file(): raise FileNotFoundError(icon)
    manifest = {"type":"preload-native","name":None,"author":None,"description":None,"version":None,"entry":"libCustomGlintColor.so","icon":"icon.png","overwrite_files":["icon.png"],"overwrite_folders":[]}
    vals = parse_version(version_header)
    for k in ('name','author','description','version'): manifest[k] = vals[k.title()]
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists(): output.unlink()
    with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        z.writestr('manifest.json', json.dumps(manifest, indent=2)+'\n')
        z.write(library, 'libCustomGlintColor.so')
        z.write(icon, 'icon.png')
    with zipfile.ZipFile(output) as z:
        names=set(z.namelist())
        if names != {'manifest.json','libCustomGlintColor.so','icon.png'}: raise RuntimeError(f'Unexpected entries: {sorted(names)}')
def main():
    p=argparse.ArgumentParser(); p.add_argument('--library',required=True,type=Path); p.add_argument('--icon',required=True,type=Path); p.add_argument('--version-header',required=True,type=Path); p.add_argument('--output',required=True,type=Path); a=p.parse_args()
    try: write_package(a.library.resolve(), a.icon.resolve(), a.version_header.resolve(), a.output.resolve())
    except Exception as e: print(e,file=sys.stderr); return 1
    print(a.output.resolve()); return 0
if __name__ == '__main__': raise SystemExit(main())
