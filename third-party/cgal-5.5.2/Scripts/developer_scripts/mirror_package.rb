#! /usr/bin/ruby

# == Synopsis
#
# "Mirrors" package files into a build folder for compilation.
#
# The package files are taken from a source package directory
# (typically a folder under an svn working copy)
#
# The directory structure under the package folder is replicated under
# a target build directory (creating subdirs as neccesary)
#
# "Mirroring" a package file consist on creating within the build directory a "view"
# to the file located into the package directory.
# This allows compilation within a "CGAL build directory structure" while
# keeping the source files in its original location within an svn working copy.
#
# In true POSIX platforms this is done via a symlink.
#
# In Windows platforms hardlinks are used instead. To ensure that broken hardlinks
# are properly syncronized, these are re-created whenever the source file is newer.
# 
# The mirroring operation is automatically selected according to the platform,
# but can be overrided if needed via the '--mirror_operation' option.
#
# Unless specified otherwise using the '-all' option, internal "dont_submit" files
# are excluded.
#
# == Usage
#
# mirror_package [OPTIONS] PACKAGE_SUBDIR BUILD_ROOT
#
# PACKAGE_SUBDIR  the source package sub directory
#
# BUILD_ROOT  the destination build root directory
#
# OPTIONS:
#
# -h, --help:
#    show help
#
# -o, --mirror_operation [symlink|hardlink]
#    Mirroring operation
#       symlink  is the default in platforms supporting it (*nix)
#       hardlink is the default in platforms not supporting symlinks (windows)
#    
# -i, --include_internal
#    Include internal files (excluded by default).
#

require 'getoptlong'
require 'rdoc/usage'

require 'list_package_files_impl.rb'
require 'mirror_package_impl.rb'

# -- TEST -- 
 ARGV = [TEST_PKG_ROOT, TEST_BUILD_ROOT]
# ARGV = ['-i',TEST_PKG_DIR, TEST_BUILD_ROOT]
# -- TEST -- 

opts = GetoptLong.new( [ '--help'            , '-h', GetoptLong::NO_ARGUMENT ],
                       [ '--mirror_operation', '-o', GetoptLong::REQUIRED_ARGUMENT ],
                       [ '--include_internal', '-i', GetoptLong::NO_ARGUMENT ]
                     )

include_internal = false

mirror_operation = default_mirror_op

opts.each do |opt, arg|
  case opt
    when '--help'
      RDoc::usage
      
    when '--mirror_operation'
      case arg
        when 'symlink'
          mirror_operation = :symlink
        when 'hardlink'
          mirror_operation = :hardlink
        else
          $stderr << "Invalid mirror operation: " << arg
      end      
      
    when '--include_internal'
      include_internal = true
       
  end
end

src_package_subdir = ARGV.length >= 1 ? ARGV.shift : ''
tgt_build_root     = ARGV.length >= 1 ? ARGV.shift : ''

unless src_package_subdir.empty? || tgt_build_root.empty? then

  files = list_package_files(src_package_subdir,include_internal)

  mirror_package(files,src_package_subdir,tgt_build_root,mirror_operation)
  
else
  RDoc::usage
end
