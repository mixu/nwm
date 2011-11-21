import sys

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

def build(bld):
  # nwm.cc
  if sys.platform.startswith("darwin"):
    obj = bld.new_task_gen('cxx', 'shlib')
    obj.linkflags=[ '-L/usr/X11/lib']
  else:
     obj = bld.new_task_gen('cxx', 'shlib', framework=['X11','Xinerama'])
  obj.lib=['X11', 'Xinerama']
  obj.cxxflags = ["-g", "-static", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
  obj.target = "nwm_base"
  obj.source = "./src/nwm.cc"

  # nwm_node.cc
  if sys.platform.startswith("darwin"):
    nwmnodetask = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    nwmnodetask.linkflags=[ '-L/usr/X11/lib']
  else:
     nwmnodetask = bld.new_task_gen('cxx', 'shlib', 'node_addon', framework=['X11'])
  nwmnodetask.lib=['X11', 'ev']
  nwmnodetask.cxxflags = ["-g", "-static", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
  nwmnodetask.target = "nwm"
  nwmnodetask.source = "./src/nwm_node.cc"
