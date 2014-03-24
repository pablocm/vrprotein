#! /usr/bin/ruby

@@unknown_elems = []

# Pasar archivo PDB por conversor SURF
def main(filename)
	in_file = File.open(filename, 'r')
	temp_filename = "#{filename}_#{Time.now.to_i}"
	temp_file = File.new(temp_filename, 'w')
	#out_filename = "#{filename}_#{Time.now.to_i}.out"

	puts "Opening #{in_file.path}..."
	count = 0
	in_file.each do |line|
		if line.start_with? 'ATOM  '
			temp_file.puts [
				count += 1,		# atom_id
				radius_for(line[12, 15]),	#atom_radius
				line[30, 37].to_f,	# atom_center_x
				line[38, 45].to_f,	# atom_center_y
				line[46, 53].to_f,	# atom_center_z
				].join ' '
		end
	end
	temp_file.close
	in_file.close
	puts "Loaded #{count} atoms."
	puts "Running: tools/surf_LINUXAMD64 -W 1 -R 1.4 #{temp_filename}"
	surf_output = `tools/surf_LINUXAMD64 -W 1 -R 1.4 #{temp_filename}`
	puts surf_output
	puts "Surf process finished."
	
	File.delete(temp_filename)
end

def radius_for(name)
	elem = name.upcase[/[A-Z]/]
	case elem
		when 'H' then return 1.0
		when 'C' then return 1.5
		when 'N' then return 1.4
		when 'O' then return 1.3
		when 'F' then return 1.2
		when 'S' then return 1.9
		else
			unless @@unknown_elems.include? elem
				puts "Warning: Unknown radius for element '#{elem}'. Using default: 1.5"
				@@unknown_elems << elem
			end
			return 1.5
	end
end

if ARGV[0]
	main(ARGV[0])
else
	puts 'Usage: ./convert_surf.rb <pdb_file>'
end