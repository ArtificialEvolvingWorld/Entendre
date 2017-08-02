Import('env')

env['CC'] = '/opt/intel/compilers_and_libraries_2017.4.196/linux/bin/intel64/icc'
env['CXX'] = '/opt/intel/compilers_and_libraries_2017.4.196/linux/bin/intel64/icc'


#env.OptionalCUDA()

env.Append(CPPPATH=[Dir('include').RDirs('.')])
env.Append(CPPPATH=['glog/include'])
env.SharedLibraryDir('glog')

env.SharedLibraryDir('libFeedForward')
env.SharedLibraryDir('libEntendre', dependencies=['glog'])
env.SharedLibraryDir('libNeat', dependencies=['Entendre'])
env.PythonLibraryDir('pyneat.so', 'libpyneat', dependencies=['Neat','Entendre'])
env.UnitTestDir('entendre_tests','tests', extra_inc_dir='include')
env.MainDir('.')
