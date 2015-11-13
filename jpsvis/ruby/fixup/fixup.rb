require "fileutils"

class BundleFixer
  def initialize(bundle)
    @bundle_dir = bundle.sub(%r{/$},"")
    raise "No such bundle #{bundle}" unless File.directory? @bundle_dir
    basename = File.basename(@bundle_dir, ".app")
    @embed_dir = "#{@bundle_dir}/Contents/Frameworks"
    @embed_rel_dir = "@executable_path/../Frameworks"

    @job = Dir["#{@bundle_dir}/Contents/MacOS/*"].select{|path| File.executable? path} + Dir["#{@bundle_dir}/Contents/PlugIns/**/*.dylib"]

    # ignore prerequisites matching these patterns
    @ignore_patterns = [
      # system libraries and frameworks
      %r{^/usr/lib},
      %r{^/System},
      # already fixed links
      %r{@executable_path}
    ]
  end

  def fixup
    @copy_hash = {}
    @rename_hashes = Hash.new {|h,k| h[k]={}}

    puts "Scanning for prerequisites..."
    @job.each do |path|
      puts " - #{path}"
      scan path
    end
    puts "Copying prerequisites into the bundle..."
    copy
    puts "Fixing link names..."
    fix_names
  end

  # scan given object file for prerequisites (using otool -L) and collect them into data structures:
  # @copy_hash maps absolute path to embedded path (relative to the bundle's @embed_dir)
  # @rename_hashes maps each embedded path to a hash that maps old names (absolute or using @loader_path) to new names (using @executable_path)
  def scan object, embedded_object=object
    context = File.dirname(object)
    IO.popen "otool -L #{object}" do | lines |
      lines.each do | l |
        l.match( %r{\t([^ ]+) \(compatibility} ) do | m |
          old_name = m[1]
          unless ignore? old_name
            if File.basename(old_name) == File.basename(object) then
              @rename_hashes[embedded_object][old_name] = object.sub(%r{#{@bundle_dir}/Contents/},"@executable_path/../")
            else
              absolute_path = make_absolute_path old_name, context
              copy_src = absolute_path
              embedded_path = File.basename absolute_path
              absolute_path.match( %r{(.+)\.framework(/.*/\1)} ) do | m |
                copy_src = absolute_path[0, m.offset(2)[0]]
              embedded_path = m[0]
              end
              @rename_hashes[embedded_object][old_name] = "#{@embed_rel_dir}/#{embedded_path}"
              unless @copy_hash.key? copy_src
                @copy_hash[copy_src] = File.basename copy_src
                scan absolute_path, embedded_path
              end
            end
          end
        end
      end
    end
  end

  # copy prerequisites from @copy_hash
  def copy
    FileUtils.mkdir_p @embed_dir
    @copy_hash.each do |absolute,embedded|
      dst = "#{@embed_dir}/#{embedded}"
      unless File.exists? dst
        puts "copy #{File.basename absolute}"
        unless system "cp -a #{absolute} #{dst}"
          puts "Error copying #{absolute} to #{dst}"
        end
      end
    end
  end

  # rewrite object id's and names under which they refer to each other
  def fix_names
    @rename_hashes.each do |basename,h|
      puts "fixing #{basename}"
      changes = h.map{|old,new| "\\\n -change #{old} #{new}"}
      if @job.include?(basename)
        path =  basename
        new_name = basename.sub(%r{#{@bundle_dir}/Contents/},"@executable_path/../")
      else
        path = "#{@embed_dir}/#{basename}"
        new_name = "#{@embed_rel_dir}/#{basename}"
      end
      system "chmod u+w #{path}"
      cmd = "install_name_tool \\\n -id #{new_name} #{changes.join(' ')} \\\n #{path}"
      unless system cmd
        puts "Error fixing names in #{path}"
        puts "Command line was:\n#{cmd}"
      end
    end
  end

  def ignore? id
    @ignore_patterns.any? { |re| id.match(re) }
  end

  def make_absolute_path path, context
    # resolve @loader_path references
    path = path.sub("@loader_path", context)
    # follow symlinks
    File.realpath path
  end

end


ARGV.each do |bundle|
  f = BundleFixer.new bundle
  f.fixup
end

