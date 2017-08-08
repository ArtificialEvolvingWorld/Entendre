Import('env')

#env.OptionalCUDA()

env.Append(CPPPATH=[Dir('include').RDirs('.')])
env.Append(CPPPATH=['glog/include'])
env.SharedLibraryDir('glog')
env.Append(CPPPATH=['cereal/include'])
env.Append(CPPPATH=['/opt/cray/pe/mpt/7.6.0/gni/mpich-gnu/5.1/include'])
env.Append(LIBPATH=['/opt/cray/pe/mpt/7.6.0/gni/mpich-gnu/5.1/lib'])
env.Append(LIBS=[ 'libmpichcxx.so','libmpich.so'])

env.SharedLibraryDir('libFeedForward')
env.SharedLibraryDir('libEntendre', dependencies=['glog'])
env.SharedLibraryDir('libNeat', dependencies=['Entendre'])
env.PythonLibraryDir('pyneat.so', 'libpyneat', dependencies=['Neat','Entendre'])
env.UnitTestDir('entendre_tests','tests', extra_inc_dir='include')
env.MainDir('.')
