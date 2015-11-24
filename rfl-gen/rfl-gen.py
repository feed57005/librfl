#!/usr/bin/env python

import os
import sys
import argparse
import rfl.proto

from importlib import import_module


def Main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-g', '--generator', required=True,
                        help='Generator module')
    parser.add_argument('-i', '--inputs', nargs='+',
                        help='Input files')
    parser.add_argument('-o', '--output-dir',
                        help='Output directory')
    parser.add_argument('-p', '--print-files', action='store_true',
                        help='Print output files')
    parser.add_argument('--pkg-name',
                        help='Package Name')
    parser.add_argument('--pkg-version',
                        help='Package Version')
    parser.add_argument('--plugin', action='store_true',
                        help='Generate plugin')
    args = parser.parse_args()

    generator_module_name = os.path.basename(args.generator)
    if os.path.isabs(args.generator):
        generator_path = os.path.dirname(args.generator)
    else:
        generator_path = os.path.abspath(os.path.dirname(args.generator))

    sys.path.append(generator_path)
    generator_module = None
    try:
        generator_module = import_module(generator_module_name)
    except ImportError as e:
        print e, generator_path, generator_module_name
        return 1

    if not args.print_files:
        pkg = rfl.proto.Package()
        print 'merging protos', args.inputs
        for proto in args.inputs:
            pb = open(proto, 'rb').read()
            input = rfl.proto.Package.FromString(pb[4:])
            if not pkg.name or pkg.name == input.name:
                pkg.MergeFrom(input)
            else:
                print 'Package names does not match', input.name, pkg.name
                return 1

        with rfl.generator.CreateContext(generator_module.Factory, args) as ctx:
            package = ctx.CreatePackage(pkg)
            generator = ctx.CreateGenerator()
            generator.Generate(package)
    else:
        with rfl.generator.CreateContext(generator_module.Factory, args) as ctx:
            generator = ctx.CreateGenerator()
            outs = generator.GetOutputFiles(args.pkg_name, args.pkg_version,
                                            args.inputs)
            for out in outs:
                print out

    return 0


if __name__ == '__main__':
    sys.exit(Main())
