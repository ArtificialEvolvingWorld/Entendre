Import('env')

env['CC'] = 'icc'
env['CXX'] = 'icc'

env.Append(CCFLAGS=['-qopenmp'])

env.Append(LIBPATH=['/opt/intel/vtune_amplifier_xe/lib64'])
env.Append(LINKFLAGS=['-qopenmp'])
env.Append(LIBS=['-littnotify'])
env.Append(CPPPATH=[Dir('include').RDirs('.')])
env.Append(CPPPATH=['glog/include'])
env.Append(CPPPATH=['/opt/intel/vtune_amplifier_xe/include'])

env.SharedLibraryDir('glog')

env.SharedLibraryDir('libFeedForward')
env.SharedLibraryDir('libEntendre', dependencies=['glog','ittnotify'])
env.SharedLibraryDir('libNeat', dependencies=['Entendre'])
env.PythonLibraryDir('pyneat.so', 'libpyneat', dependencies=['Neat','Entendre'])
env.UnitTestDir('entendre_tests','tests', extra_inc_dir='include')
env.MainDir('.')
