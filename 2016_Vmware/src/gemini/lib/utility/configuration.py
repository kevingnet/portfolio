# Kevin Guerra:

"""Configuration class

- Load configuration
- Merge a second configuration (local) with more specific values
- Replace entries from ENV variables

"""
import os.path
import re
import unicodedata
import yaml
from lib.service.run_command import runCommand
from lib.utility.dict_as_member import DottedNotation
from lib import PARENT_DIR

# yaml custom constructor to process ENV_VARS
pattern = re.compile(r'^\$(.*)$')
yaml.add_implicit_resolver ("!pathex", pattern)
yamlFile = '/tmp/configuration.yaml'


def pathex_constructor(loader, node):
    value = loader.construct_scalar(node)
    envVar = pattern.match(value).groups()[0]
    if os.environ.has_key(envVar):
        return os.environ[envVar]
    envVar = '$' + unicodedata.normalize\
        ('NFKD', envVar).encode('ascii', 'ignore')
    return envVar

yaml.add_constructor('!pathex', pathex_constructor)


class Configuration(object):

    def __init__(self, config_name):
        if (os.path.isfile(config_name) == True):
            config_stream = open(config_name, 'r')
            self.config = yaml.load(config_stream)
            self.normalize_dictionary(self.config)
            self.config = DottedNotation(self.config)
        else:
            raise OSError(config_name)

    def dump(self, filePath):
        self.config.update()
        with open(filePath, 'w') as outfile:
            outfile.write(yaml.dump(self.config, default_flow_style=False))
        # print yaml.dump(self.config, default_flow_style=False)
        # assert 1==0

    # keys that start with a '$' are substituted if
    # a corresponding root key is found
    def normalize_dictionary(self, d):
        for k, v in d.iteritems():
            if isinstance(v, dict):
                self.normalize_dictionary(v)
            elif isinstance(v, list):
                pass
            else:
                try:
                    if v is not None and v[0] is not None and (v[0] == '$'):
                        d[k] = self.config[v]
                except:
                    pass

    def writeConfiguration(self):
        yamlConfiguration = os.path.join(self.testDirectory,
                                         'configuration.yaml')
        self.dump(yamlConfiguration)


def consolidate_files(testPath):
    # consolidate configuration files
    configurationYaml = os.path.join(PARENT_DIR, 'configuration.yaml')
    localYaml = os.path.join(PARENT_DIR, 'local.yaml')
    suiteYaml = os.path.dirname(testPath) + "/suite.yaml"
    testYaml = testPath + ".yaml"
    yamlFiles = []
    if (os.path.isfile(configurationYaml) == True):
        yamlFiles.append(configurationYaml)
    else:
        raise OSError(configurationYaml)
    if (os.path.isfile(localYaml) == True):
        yamlFiles.append(localYaml)
    else:
        raise OSError(localYaml)
    if suiteYaml is not None and (os.path.isfile(suiteYaml) == True):
        yamlFiles.append(suiteYaml)
    if testYaml is not None and (os.path.isfile(testYaml) == True):
        yamlFiles.append(testYaml)
    with open(yamlFile, 'w') as outfile:
        for fname in yamlFiles:
            with open(fname) as infile:
                outfile.write(infile.read())
    return yamlFile
