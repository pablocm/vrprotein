#! /usr/bin/ruby
# Converts simple triangle format created by SURF to an indexed triangle
# list (reusing vertices and reducing file size) 

# Save vertices as (unique) keys in dictionary
# Values are number of repetitions (used for stats)
def parse_vertices(in_file)
	vertices = {}
	vertices.default = 0
	regex = /^(-?\d+\.\d+ ?){6}/ #3 coords & 3 normals
	
	in_file.each do |line|
		if regex.match(line)
			vertices[line] += 1
		end
	end
	vertices
end

def print_stats(vertices)
	values = vertices.values
	sum = values.reduce(:+).to_f
	puts "Total vertices:         #{sum}"
	puts "Total unique vertices:  #{values.length}"
	puts "Max repetitions:        #{values.max}"
	puts "Average repetitions:    #{sum / values.size}\n"
end

def main(filename)
	in_file = File.open(filename, 'r')
	puts "Opened #{in_file.path}..."

	vertices = parse_vertices(in_file)
	puts "Vertices extracted."

	# Stats
	print_stats(vertices)

	out_file = File.new(filename + '.idx', 'w')
	puts "Writing to #{out_file.path}..."

	# Save vertices in file (and create vertices IDs)
	id = 0
	out_file.puts "VERTICES"
	vertices.each_key do |v|
		out_file.puts "#{id} #{v}"
		vertices[v] = id
		id += 1
	end

	# Save optimized triangles in file
	number_regex = /^\d+$/
	vertices.default = nil # Debug
	out_file.puts "TRIANGLES"
	in_file.rewind
	in_file.each do |line|
		if number_regex.match(line) # Atom ids
			out_file.puts "#{line}"
		else # Coords
			v = vertices[line]
			raise "Error processing: #{line}" unless v.is_a? Fixnum
			out_file.puts "#{v}"
		end
	end

	puts "Done."

	out_file.close
	in_file.close
end

if ARGV[0]
	main(ARGV[0])
else
	puts 'Usage: ./surf_tri_to_idx_tri.rb <tri_file>'
end