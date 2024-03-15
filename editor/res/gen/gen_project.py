import os
import argparse
import uuid
import xml.etree.ElementTree as ET


class ProjectGenerator:
    def __init__(self):
        self.__path = ''
        self.__name = ''
        self.__copy_path = ''
        self.__coredll_path = ''
        self.__guid = ''
        self.__genName = 'GenProj'

    def __parse_args(self):
        parser = argparse.ArgumentParser()
        parser.add_argument('-p', '--path', help='path to generate', required=True)
        parser.add_argument('-n', '--name', help='project name', required=True)
        parser.add_argument('--copypath', help='copy bins to custom path', required=True)
        parser.add_argument('--coredllpath', help='copy robot2D core dll to custom path', required=True)

        args = parser.parse_args()
        self.__path = args.path
        self.__name = args.name
        self.__copy_path = args.copypath
        self.__coredll_path = args.coredllpath

    def __patch_sln(self, run_path):
        gen_uuid = 'FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF'
        gen_uuid_len = len(gen_uuid)
        extension = '.sln'
        try:
            lines = []
            new_lines = []
            full_name = self.__genName + extension
            full_path = os.path.join(run_path, full_name)
            with(open(full_path, 'r')) as ioreader:
                lines = ioreader.readlines()
                ioreader.close()

            for line in lines:
                new_line = line
                if line.find(self.__genName):
                    new_line = line.replace(self.__genName, self.__name)

                if line.find(gen_uuid):
                    new_line = new_line.replace(gen_uuid, self.__guid, gen_uuid_len)
                    new_lines.append(new_line)
                else:
                    new_lines.append(new_line)

            full_name = self.__name + extension
            full_path = os.path.join(run_path, full_name)
            with(open(full_path, 'w')) as iowriter:
                iowriter.writelines(new_lines)
                iowriter.close()
        except:
            pass

    def __patch_csproj(self, run_path):
        msbuild_scheme = 'http://schemas.microsoft.com/developer/msbuild/2003'
        msbuild_scheme_tag = '{http://schemas.microsoft.com/developer/msbuild/2003}'
        extension = '.csproj'

        full_name = self.__genName + extension
        full_path = os.path.join(run_path, full_name)

        tree = ET.parse(full_path)
        ET.register_namespace('', msbuild_scheme)
        root = tree.getroot()

        for child in root:
            if child.tag == msbuild_scheme_tag + 'ItemGroup':
                for c in child:
                    if c.tag == msbuild_scheme_tag + 'Reference':
                        for custom in c:
                            if custom.tag == msbuild_scheme_tag + 'HintPath':
                                custom.text = self.__coredll_path
            if child.tag == msbuild_scheme_tag + 'PropertyGroup':
                for c in child:
                    if c.tag == msbuild_scheme_tag + 'ProjectGuid':
                        c.text = '{' + self.__guid + '}'
                    if c.tag == msbuild_scheme_tag + 'RootNamespace':
                        c.text = self.__name
                    if c.tag == msbuild_scheme_tag + 'AssemblyName':
                        c.text = self.__name
            if child.tag == msbuild_scheme_tag + 'Target':
                for c in child:
                    if c.tag == msbuild_scheme_tag + 'PropertyGroup':
                        for custom in c:
                            if custom.tag == msbuild_scheme_tag + 'AfterOutputPath':
                                custom.text = self.__copy_path

        full_name = self.__name + extension
        full_path = os.path.join(run_path, full_name)
        tree.write(full_path, encoding="utf-8", xml_declaration=True)

    def __gen_proj(self, run_path):
        import shutil
        try:
            slnFullName = self.__name + '.sln'
            csprojFullName = self.__name + '.csproj'

            if not os.path.exists(self.__path):
                os.mkdir(self.__path)

            copyFullPath = os.path.join(run_path, slnFullName)
            shutil.copyfile(copyFullPath, os.path.join(self.__path, slnFullName))
            os.remove(copyFullPath)

            copyFullPath = os.path.join(run_path, csprojFullName)
            shutil.copyfile(copyFullPath, os.path.join(self.__path, csprojFullName))
            os.remove(copyFullPath)
        except:
            pass

    def run(self):
        run_path = os.path.dirname(__file__)
        self.__parse_args()
        self.__guid = str(uuid.uuid4()).upper()
        self.__patch_sln(run_path)
        self.__patch_csproj(run_path)
        self.__gen_proj(run_path)


if __name__ == '__main__':
    projGen = ProjectGenerator()
    projGen.run()
