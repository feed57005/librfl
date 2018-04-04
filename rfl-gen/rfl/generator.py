# Copyright (c) 2015 Pavel Novy. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import errno
import rfl
import platform
from jinja2 import Environment, ChoiceLoader, PackageLoader


def AnnotationToDict(anno):
    ret = {}
    for entry in anno.entries:
        ret[entry.key] = entry.value
    return ret


def QualifiedCXXNameToRfl(name):
    components = name.split('::')
    return '.'.join(components)


def PlatformLibForName(name):
    os_system = platform.system()
    library = None
    if os_system == 'Darwin':
        library = 'lib%s.dylib' % name
    elif os_system == 'Linux':
        library = 'lib%s.so' % name
    elif os_system == 'Windows':
        library = '%s.dll' % name
    return library


class Method(object):
    def __init__(self, proto, parent):
        super(Method, self).__init__()
        self.proto = proto
        self.parent = parent
        self.annotation = AnnotationToDict(proto.annotation)


class Field(object):
    def __init__(self, proto, parent):
        super(Field, self).__init__()
        self.proto = proto
        self.parent = parent
        self.annotation = AnnotationToDict(proto.annotation)


class Enum(object):
    def __init__(self, proto, parent):
        super(Enum, self).__init__()
        self.proto = proto
        self.parent = parent
        self.annotation = AnnotationToDict(proto.annotation)
        pkg_file_klass = rfl.generator.context.factory.PackageFile()

        self.full_name = self.proto.name
        p = parent
        names = [proto.name]
        while p and not isinstance(p, pkg_file_klass):
            names.insert(0, p.proto.name)
            p = p.parent
        self.full_name = '.'.join(names)
        self.qualified_name = '::'.join(names)
        self.package_file = p

        # Build namespace list
        if isinstance(parent, Class):
            ns = parent.parent
        else:
            ns = parent
        ns_klass = rfl.generator.context.factory.Namespace()
        self.namespace = []
        while ns:
            self.namespace.insert(0, ns.proto.name)
            if ns.parent.__class__ == ns_klass:
                ns = ns.parent
            else:
                ns = None


class TypeContainer(object):
    def __init__(self, proto, parent):
        super(TypeContainer, self).__init__()
        self.proto = proto
        self.parent = parent

        self.classes = []
        self.enums = []

        for klass in proto.classes:
            class_klass = rfl.generator.context.factory.Class()
            self.classes.append(class_klass(klass, self))

        for enm in proto.enums:
            enm_klass = rfl.generator.context.factory.Enum()
            self.enums.append(enm_klass(enm, self))

        pkg_file_klass = rfl.generator.context.factory.PackageFile()

        if self.__class__ != pkg_file_klass:
            p = parent
            names = [proto.name]
            while p and not isinstance(p, pkg_file_klass):
                names.insert(0, p.proto.name)
                p = p.parent
            self.full_name = '.'.join(names)
            self.qualified_name = '::'.join(names)
            self.package_file = p

    def package_file(self):
        return self.package_file


class Class(TypeContainer):
    def __init__(self, proto, parent):
        super(Class, self).__init__(proto, parent)

        self.kind = proto.annotation.kind
        self.annotation = AnnotationToDict(proto.annotation)

        # Build namespace list
        if isinstance(parent, Class):
            # TODO while parent is class
            ns = parent.parent
        else:
            ns = parent
        ns_klass = rfl.generator.context.factory.Namespace()
        self.namespace = []
        while ns:
            self.namespace.insert(0, ns.proto.name)
            if ns.parent.__class__ == ns_klass:
                ns = ns.parent
            else:
                ns = None

        self.fields = []
        for field in proto.fields:
            field_klass = rfl.generator.context.factory.Field()
            self.fields.append(field_klass(field, self))

        self.methods = []
        for method in proto.methods:
            method_klass = rfl.generator.context.factory.Method()
            self.methods.append(method_klass(method, self))


class Function(object):
    def __init__(self, proto, parent):
        super(Function, self).__init__()
        self.proto = proto
        self.parent = parent
        self.annotation = AnnotationToDict(proto.annotation)

        pkg_file_klass = rfl.generator.context.factory.PackageFile()
        p = parent
        names = [proto.name]
        while p and not isinstance(p, pkg_file_klass):
            names.insert(0, p.proto.name)
            p = p.parent
        self.full_name = '.'.join(names)
        self.qualified_name = '::'.join(names)
        self.package_file = p


class Namespace(TypeContainer):
    def __init__(self, proto, parent):
        super(Namespace, self).__init__(proto, parent)
        self.namespaces = []

        ns_klass = rfl.generator.context.factory.Namespace()
        for ns in proto.namespaces:
            self.namespaces.append(ns_klass(ns, self))

        self.functions = []
        for func in proto.functions:
            function_klass = rfl.generator.context.factory.Function()
            self.functions.append(function_klass(func, self))


class PackageFile(Namespace):
    def __init__(self, proto, parent):
        self.h_includes = set()
        self.src_includes = set()
        super(PackageFile, self).__init__(proto, parent)


class Package(object):
    def __init__(self, proto):
        super(Package, self).__init__()
        self.types = []
        self.proto = proto
        self.package_files = []
        self.h_includes = set()
        self.src_includes = set()
        for pkg_file in proto.package_files:
            pkg_file_klass = rfl.generator.context.factory.PackageFile()
            self.package_files.append(pkg_file_klass(pkg_file, self))
        self.library = PlatformLibForName(proto.name)
        self.imports = []
        for imp in proto.imports:
            self.imports.append((imp, PlatformLibForName(imp)))

    def GetPackageContents(self):
        """
        Traverse PackageFile and namespaces for classes, enums and functions.
        """
        klasses = []
        enums = []
        funcs = []
        for pkg_file in self.package_files:
            for klass in pkg_file.enums:
                enums.append(klass)
            for klass in pkg_file.classes:
                klasses.append(klass)
            for func in pkg_file.functions:
                funcs.append(func)
            for ns in pkg_file.namespaces:
                stack = []
                stack.insert(0, ns)
                while stack:
                    current = stack.pop(0)
                    for klass in current.classes:
                        if len(klass.classes):
                            stack[0:0] = [klass]
                        klasses.append(klass)
                    for enm in current.enums:
                        enums.append(enm)
                    for func in current.functions:
                        funcs.append(func)
                    if hasattr(current, 'namespaces') and current.namespaces:
                        stack[0:0] = current.namespaces
        klasses = self.SortClasses(klasses)
        return enums, klasses, funcs

    def SortClasses(self, klasses):
        klass_dict = \
            {klass.qualified_name: [klass, 0] for klass in klasses}
        order = 1
        stack = []
        for klass in klasses:
            stack.insert(0, klass.qualified_name)
            while stack:
                k = klass_dict[stack.pop(0)][0]
                if klass_dict[k.qualified_name][1] == 2:
                    continue
                klass_dict[k.qualified_name][1] = 1
                if (not k.proto.HasField('base_class') or
                    (k.proto.base_class.type_name in klass_dict and
                     klass_dict[k.proto.base_class.type_name][1])):
                    k.proto.order = order
                    klass_dict[k.qualified_name][1] = 2
                    order += 1
                    continue

                if k.proto.base_class.type_name in klass_dict:
                    stack.insert(0, k.qualified_name)
                    stack.insert(0, k.proto.base_class.type_name)
                    continue

                visit_fields = []
                for field in k.fields:
                    if (field.type_name in klass_dict and not
                            klass_dict[field.type_name][1]):
                        visit_fields.append(field.type_name)
                if visit_fields:
                    stack.insert(0, k.qualified_name)
                    stack = visit_fields + stack
                    continue
                else:
                    klass_dict[k.qualified_name][1] = 2
                    k.proto.order = order
                    order += 1
                    continue

        klasses.sort(key=lambda x: x.proto.order)
        return klasses

################################################################################

_stack = []


class Generator(object):
    def __init__(self, out_dir):
        super(Generator, self).__init__()
        self.package = None
        self.output_dir = out_dir

    def Generate(self, pkg):
        self.package = pkg
        for pkg_file in pkg.package_files:
            self.GenerateFile(pkg_file)
        self.GeneratePackage(pkg)

    def _CreateJinjaEnv(self):
        # module = self.__module__.split('.')[:-1]
        loader = ChoiceLoader([
            PackageLoader(self.__module__)
            # PackageLoader('rfl'),
            # PackageLoader(".".join(module))
            ])
        env = Environment(loader=loader,
                          extensions=["jinja2.ext.do"])
        return env

    def _Save(self, fname, content):
        try:
            fullpath = os.path.abspath(os.path.join(self.output_dir, fname))
            os.makedirs(os.path.dirname(fullpath))
        except OSError as err:
            if err.errno != errno.EEXIST:
                print("Cannot create directory \"",
                      os.path.dirname(fname), "\"")
                exit(-1)

        with open(fullpath, "w") as fout:
            fout.write(content)

    def GenerateFile(self, pkg_file):
        raise NotImplemented("Must be overriden")

    def GeneratePackage(self, pkg):
        raise NotImplemented("Must be overriden")

    def GetOutputFiles(self, name, version, inputs):
        raise NotImplemented("Must be overriden")


class Factory(object):
    @classmethod
    def Package(cls):
        return Package

    @classmethod
    def PackageFile(cls):
        return PackageFile

    @classmethod
    def Class(cls):
        return Class

    @classmethod
    def Field(cls):
        return Field

    def Method(cls):
        return Method

    @classmethod
    def Enum(cls):
        return Enum
    @classmethod
    def Function(cls):
        return Function

    @classmethod
    def Namespace(cls):
        return Namespace

    def Generator(cls):
        return Generator


class Context(object):
    def __init__(self, factory):
        super(Context, self).__init__()
        self.output_dir_ = None
        self.factory_ = factory

    @property
    def factory(self):
        return self.factory_

    @property
    def output_dir(self):
        return self.output_dir_

    def CreatePackage(self, pkg_proto):
        pkg_klass = self.factory.Package()
        return pkg_klass(pkg_proto)

    def CreateGenerator(self):
        generator_klass = self.factory.Generator()
        return generator_klass(self.args.output_dir)

    def __enter__(self):
        _stack.append(self)
        rfl.generator.context = self
        return self

    def __exit__(self, *_):
        _stack.pop()
        if _stack:
            rfl.generator.context = _stack[-1]
        return False


def CreateContext(factory, args):
    ctx = Context(factory)
    ctx.args = args
    return ctx
