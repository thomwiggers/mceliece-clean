import glob
import os
from typing import Optional, List
from functools import lru_cache

import yaml
import platform
import helpers


class Scheme:
    def __init__(self):
        self.type = None
        self.name = None
        self.implementations = []

    def path(self, base='..'):
        return os.path.join(base, 'crypto_' + self.type, self.name)

    def namespace_prefix(self):
        return 'PQCLEAN_{}_'.format(self.name.upper()).replace('-', '')

    @staticmethod
    @lru_cache(maxsize=None)
    def by_name(scheme_name):
        for scheme in Scheme.all_schemes():
            if scheme.name == scheme_name:
                return scheme
        raise KeyError()

    @staticmethod
    @lru_cache(maxsize=1)
    def all_schemes():
        schemes = []
        schemes.extend(Scheme.all_schemes_of_type('kem'))
        schemes.extend(Scheme.all_schemes_of_type('sign'))
        return schemes

    @staticmethod
    @lru_cache(maxsize=1)
    def all_implementations():
        implementations = []
        for scheme in Scheme.all_schemes():
            implementations.extend(scheme.implementations)
        return implementations

    @staticmethod
    @lru_cache(maxsize=1)
    def all_supported_implementations():
        return [impl for impl in Scheme.all_implementations()
                if impl.supported_on_current_platform()]

    @staticmethod
    @lru_cache(maxsize=32)
    def all_schemes_of_type(type: str) -> list:
        schemes = []
        p = os.path.join('..', 'crypto_' + type)
        if os.path.isdir(p):
            for d in os.listdir(p):
                if os.path.isdir(os.path.join(p, d)):
                    if type == 'kem':
                        schemes.append(KEM(d))
                    elif type == 'sign':
                        schemes.append(Signature(d))
                    else:
                        assert('Unknown type')
        return schemes

    @lru_cache(maxsize=None)
    def metadata(self) -> Optional[dict]:
        metafile = os.path.join(self.path(), 'META.yml')
        try:
            with open(metafile, encoding='utf-8') as f:
                metadata = yaml.safe_load(f)
            return metadata
        except Exception as e:
            print("Can't open {}: {}".format(metafile, e))
            return None

    def __repr__(self):
        return "<{}({})>".format(self.type.title(), self.name)


class Implementation:

    def __init__(self, scheme, name):
        self.scheme = scheme
        self.name = name

    @lru_cache(maxsize=None)
    def metadata(self):
        for i in self.scheme.metadata()['implementations']:
            if i['name'] == self.name:
                return i

    def path(self, base='..') -> str:
        return os.path.join(self.scheme.path(base=base), self.name)

    def libname(self) -> str:
        if os.name == 'nt':
            return "lib{}_{}.lib".format(self.scheme.name, self.name)
        return "lib{}_{}.a".format(self.scheme.name, self.name)

    def cfiles(self) -> List[str]:
        return glob.glob(os.path.join(self.path(), '*.c'))

    def hfiles(self) -> List[str]:
        return glob.glob(os.path.join(self.path(), '*.h'))

    def ofiles(self) -> List[str]:
        return glob.glob(os.path.join(self.path(),
                         '*.o' if os.name != 'nt' else '*.obj'))

    @staticmethod
    @lru_cache(maxsize=None)
    def by_name(scheme_name, implementation_name):
        scheme = Scheme.by_name(scheme_name)
        for implementation in scheme.implementations:
            if implementation.name == implementation_name:
                return implementation
        raise KeyError(f"impl {implementation_name} not found")

    @staticmethod
    @lru_cache(maxsize=None)
    def all_implementations(scheme: Scheme) -> List['Implementation']:
        return [Implementation(scheme, impl['name']) for impl in scheme.metadata()['implementations']]

    @staticmethod
    def all_supported_implementations(scheme: Scheme) -> list:
        return [impl for impl in Implementation.all_implementations(scheme)
                if impl.supported_on_current_platform()]

    def namespace_prefix(self):
        return '{}{}_'.format(self.scheme.namespace_prefix(),
                              self.name.upper()).replace('-', '')

    def supported_on_os(self, os: Optional[str] = None) -> bool:
        """Check if we support the OS

        If no OS is specified, then we run on the current OS
        """
        if os is None:
            os = platform.system()
        metadata = self.metadata()
        assert metadata is not None

        for platform_ in metadata.get('supported_platforms', []):
            if 'operating_systems' in platform_:
                if os not in platform_['operating_systems']:
                    return False

        return True

    @lru_cache(maxsize=10000)
    def supported_on_current_platform(self) -> bool:
        metadata = self.metadata()
        assert metadata is not None, "Unable to find metadata to decide support"
        if 'supported_platforms' not in metadata:
            return True

        if platform.machine() == 'ppc':
            return False

        if not self.supported_on_os():
            return False

        cpuinfo = helpers.get_cpu_info()

        for platform_ in metadata['supported_platforms']:
            if platform_['architecture'] == cpuinfo['arch'].lower():
                # Detect actually running on emulated i386
                if (platform_['architecture'] == 'x86_64' and
                        platform.architecture()[0] == '32bit'):
                    continue
                if all([flag in cpuinfo['flags']
                        for flag in platform_['required_flags']]):
                    return True
        return False

    def __str__(self):
        return "{} implementation of {}".format(self.name, self.scheme.name)

    def __repr__(self):
        return "<Implementation({}, {})>".format(self.scheme.name, self.name)


class KEM(Scheme):

    def __init__(self, name: str):
        self.type = 'kem'
        self.name = name
        self.implementations = Implementation.all_implementations(self)

    @staticmethod
    def all_kems() -> list:
        return Scheme.all_schemes_of_type('kem')


class Signature(Scheme):

    def __init__(self, name: str):
        self.type = 'sign'
        self.name = name
        self.implementations = Implementation.all_implementations(self)

    @staticmethod
    def all_sigs():
        return Scheme.all_schemes_of_type('sign')
