from CI import *
import platform, subprocess

coustom_bld_toolchain_os = ['ios', 'android']

generators = {
    'windows'   : 'Visual Studio 15 2017 Win64',
    'windowsuwp': 'Visual Studio 15 2017 Win64',
    'linux'     : 'Unix Makefiles',
    'macos'     : 'Xcode',
    'ios'       : 'Xcode',
    'android'   : 'Android Gradle - Ninja'
}

def generate_cmake(config, src_dir, bld_dir, inst_dir, target_os):
    if target_os == 'android':
        from ci.android import build_android_instance
        return

    config_mapping = {
        'debug': 'Debug',
        'release': 'Release'
    }

    P = subprocess.Popen(['cmake', 
        '-G' + generators[target_os],
        '-H{0}'.format(src_dir),
        '-B{0}'.format(bld_dir),
        '-DCMAKE_BUILD_TYPE={0}'.format(config_mapping[config]),
        '-DCMAKE_INSTALL_PREFIX={0}'.format(inst_dir)
        ])
    P.wait()
    
    if P.returncode != 0:
        print('Generate Failed!')
        return

def build_cmake(config, bld_dir, need_install = False):
    config_mapping = {
        'debug': 'Debug',
        'release': 'Release'
    }

    cmd_list = ['cmake', '--build', bld_dir, '--config', config_mapping[config]]
    if need_install:
        cmd_list.append('--target')
        cmd_list.append('install')

    P = subprocess.Popen(cmd_list)
    P.wait()