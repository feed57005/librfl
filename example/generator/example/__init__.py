import rfl.generator
import os


def HeaderFilenameForSource(src):
    return src + '.rfl.h'


def SourceFilenameForSource(src):
    return src + '.rfl.cc'


class Enum(rfl.generator.Enum):
    def __init__(self, proto, parent):
        super(Enum, self).__init__(proto, parent)
        self.items = []
        for item in proto.items:
            self.items.append((item.id, item.value,
                               self.annotation.get(item.id, item.id)))
        self.package_file.h_includes.add('"example/enum_class.h"')

        if isinstance(parent, rfl.generator.Class):
            comps = self.full_name.split('.')
            comps[-2] = comps[-2] + 'Class'
            self.qualified_class_name = '::'.join(comps) + 'Class'
        else:
            self.qualified_class_name = self.qualified_name + 'Class'


class Class(rfl.generator.Class):
    def __init__(self, proto, parent):
        super(Class, self).__init__(proto, parent)

        if proto.base_class and proto.base_class.source_file:
            if self.package_file.proto.name != proto.base_class.source_file:
                self.package_file.h_includes.add('"%s.rfl.h"'
                                                 % proto.base_class.source_file)
            self.base_class = proto.base_class.type_name
            self.package_file.h_includes.add('"%s"'
                                             % proto.base_class.source_file)
        else:
            self.base_class = 'example::Object'
            self.package_file.h_includes.add('"example/object.h"')

        if self.methods:
            self.package_file.h_includes.add('"example/call_desc.h"')

        if self.proto.fields:
            self.package_file.h_includes.add('"example/property.h"')

        if isinstance(parent, rfl.generator.Class):
            comps = self.full_name.split('.')
            comps[-2] = comps[-2] + 'Class'
            self.qualified_class_name = '::'.join(comps) + 'Class'
        else:
            self.qualified_class_name = self.qualified_name + 'Class'


class Method(rfl.generator.Method):
    def __init__(self, proto, parent):
        super(Method, self).__init__(proto, parent)
        argtypes = []
        self.signature_descriptor = 'x'
        for arg in proto.arguments:
            anno = rfl.generator.AnnotationToDict(arg.annotation)
            kind = anno.get('kind', 'in')
            if kind == 'in':
                self.signature_descriptor += 'i'
            elif kind == 'out':
                self.signature_descriptor += 'o'
            else:
                self.signature_descriptor += 'a'

            argtypes.append(arg.type_ref.type_name)

        self.signature = '%s(%s::*)(%s)' % (
            proto.return_value.type_ref.type_name, parent.proto.name,
            ', '.join(argtypes))

        if 'name' not in self.annotation:
            self.annotation['name'] = proto.name


class Field(rfl.generator.Field):
    def __init__(self, proto, parent):
        super(Field, self).__init__(proto, parent)

        self.type_name = proto.type_ref.type_name

        if 'kind' not in self.annotation:
            self.annotation['kind'] = 'generic'
        self.kind = self.annotation['kind']

        if 'id' not in self.annotation:
            self.annotation['id'] = proto.name
        self.id = self.annotation['id']

        if 'name' not in self.annotation:
            self.annotation['name'] = self.id

        if self.kind == 'number':
            is_floating = \
                self.type_name == 'float' or self.type_name == 'double'
            if 'min' not in self.annotation:
                self.parent.package_file.src_includes.add('<limits>')
                self.annotation['min'] = \
                    '-(std::numeric_limits<%s>::max())' % self.type_name
            if 'max' not in self.annotation:
                self.parent.package_file.src_includes.add('<limits>')
                self.annotation['max'] = \
                    'std::numeric_limits<%s>::max()' % self.type_name
            if 'step' not in self.annotation:
                self.annotation['step'] = '0.1' if is_floating else '1'
            if 'page_step' not in self.annotation:
                self.annotation['page_step'] = '0.1' if is_floating else '1'
            if 'page_size' not in self.annotation:
                self.annotation['page_size'] = '0.1' if is_floating else '1'
            if 'precision' not in self.annotation:
                self.annotation['precision'] = 2 if is_floating else 0
        elif self.kind != 'enum':
            if proto.type_qualifier.is_pointer:
                self.any_value = "(%s *)" % self.type_name
                if 'default' not in self.annotation:
                    self.annotation['default'] = 'nullptr'
            elif proto.type_qualifier.is_const:
                self.any_value = "(%s)" % self.type_name[6:]
            else:
                self.any_value = self.type_name
            if 'default' not in self.annotation:
                self.annotation['default'] = ''
            self.any_value += '(%s)' % self.annotation['default']


class File(rfl.generator.PackageFile):
    def __init__(self, proto, parent):
        super(File, self).__init__(proto, parent)
        self.h_includes.add('"%s"' % parent.export_h)
        self.h_includes.add('"example/type_repository.h"')
        self.h_guard = '__' + \
            proto.name.replace(os.path.sep, '_').replace('-', '_').\
            replace('.', '_').upper() + '__'
        self.generated_header = HeaderFilenameForSource(proto.name)
        self.generated_source = SourceFilenameForSource(proto.name)
        self.src_includes.add('"%s"' % self.generated_header)
        self.parent.h_includes.add('"%s"' % self.generated_header)


class Package(rfl.generator.Package):
    def __init__(self, proto):
        self.package_id = proto.name.replace('-', '_')
        self.export_def = self.package_id.upper() + '_EXPORT'
        self.export_h = self.package_id.lower() + '_export.h'
        self.h_guard = '__' + self.package_id.upper() + '__'
        super(Package, self).__init__(proto)


class Generator(rfl.generator.Generator):
    def __init__(self, out_dir):
        super(Generator, self).__init__(out_dir)
        self.env = self._CreateJinjaEnv()

    def GetOutputFiles(self, name, version, inputs):
        package_id = name.lower()
        outs = [
            os.path.join(self.output_dir, HeaderFilenameForSource(package_id)),
            os.path.join(self.output_dir, SourceFilenameForSource(package_id)),
            os.path.join(self.output_dir, package_id + "_export.h"),
        ]
        for input in inputs:
            outs.append(
                os.path.join(self.output_dir, HeaderFilenameForSource(input)))
            outs.append(
                os.path.join(self.output_dir, SourceFilenameForSource(input)))
        return outs

    def GenerateFile(self, pkg_file):
        data = {'package_file': pkg_file, 'generator': self}

        pkg_h = self.env.get_template('package_file_header.tmpl')
        pkg_h_out = pkg_h.render(data)
        self._Save(pkg_file.generated_header, pkg_h_out)

        pkg_cc = self.env.get_template('package_file_source.tmpl')
        pkg_cc_out = pkg_cc.render(data)
        self._Save(pkg_file.generated_source, pkg_cc_out)

    def GeneratePackage(self, pkg):
        enums, klasses = pkg.GetPackageClasses()
        structs = filter(
            lambda x: x.kind != 'class', klasses)
        klasses = filter(
            lambda x: x.kind == 'class', klasses)
        data = {'package': pkg,
                'generator': self,
                'classes': klasses,
                'structs': structs,
                'enums': enums,
                'ctx': rfl.generator.context}

        pkg_export = self.env.get_template('package_export.tmpl')
        pkg_export_out = pkg_export.render(data)
        self._Save(pkg.package_id.lower() + '_export.h', pkg_export_out)

        pkg_h = self.env.get_template('package_header.tmpl')
        pkg_h_out = pkg_h.render(data)
        self._Save(pkg.package_id.lower() + '.rfl.h', pkg_h_out)

        pkg_cc = self.env.get_template('package_source.tmpl')
        pkg_cc_out = pkg_cc.render(data)
        self._Save(pkg.package_id.lower() + '.rfl.cc', pkg_cc_out)
        pass


class Factory(rfl.generator.Factory):
    @classmethod
    def Enum(cls):
        return Enum

    @classmethod
    def Class(cls):
        return Class

    @classmethod
    def Field(cls):
        return Field

    @classmethod
    def Method(cls):
        return Method

    @classmethod
    def PackageFile(cls):
        return File

    @classmethod
    def Package(cls):
        return Package

    @classmethod
    def Generator(cls):
        return Generator
