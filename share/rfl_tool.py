import os
import sys
import time
from subprocess import call


def generate_compdb(args):
    out = args[0]
    with open(out, 'w') as outfile:
        call(["ninja", "-t", "compdb", "cxx"], stdout=outfile)
        outfile.flush()
        outfile.close()
    return 0


def scan(args):
    build_root = args[0]
    args = args[1:]
    ninja_file = os.path.join(build_root, "build.ninja")
    ninja_timestamp = time.ctime(os.path.getmtime(ninja_file))
    comp_db_file = os.path.join(build_root, "compile_commands.json")
    if not os.path.exists(comp_db_file):
        print 'compilation db does not exists'
        return 1
    #else:
    #    comp_db_timestamp = time.ctime(os.path.getmtime(comp_db_file))
    #    if (ninja_timestamp > comp_db_timestamp):
    #        print 'ReGenerating compilation db'
    #        generate_compdb([comp_db_file])

    rfl_scan_exe = os.path.join(os.path.dirname(__file__), os.pardir, os.pardir,
                                'bin', 'rfl-scan')
    args.insert(0, rfl_scan_exe)
    ret = call(args)
    #print "Exit: ", ret
    return ret


def depfile(args):
    depfilename = args[0]
    print 'Generating', depfilename
    generator_lib = args[1]
    with open(depfilename, 'w') as f:
        for dep in args[2:]:
            f.write(dep + ": " + generator_lib + "\n")
    return 0


cmds = {
    'generate_compdb': generate_compdb,
    'scan': scan,
    'depfile': depfile,
}


def main(argv):
    if len(argv) < 3:
        return 1

    cmd = argv[1]
    if cmd in cmds:
        return cmds[cmd](argv[2:])
    else:
        print 'Unknown stuff: ', argv
        return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
