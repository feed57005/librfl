import rfl.generator

class Generator(rfl.generator.Generator):
    def __init__(self, out_dir):
        super(Generator, self).__init__(out_dir)
        self.env = _CreateJinjaEnv()

    def GenerateFile(self, pkg_file):
        pass

    def GeneratePackage(self, pkg):

class Factory(rfl.generator.Factory):
    def Generator(cls):
        return Generator
