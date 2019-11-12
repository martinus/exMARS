# Converts from element to arrayindex and vice versa
class Converter
	def initialize(array)
		@arr = array
		@ref = Hash.new(false)
		@arr.each_index do |i|
			@ref[@arr[i]] = i
		end
	end

	def encode(string)
		self[string]
	end
	
	def decode(value)
		@arr[value]
	end	
	
	def [](x)
		@ref[x]
	end
end


# A corewars individual
class Warrior
	attr_accessor :instructions, :startPos, :name, :authorName, :fileName, :id
	
	
	def initialize
		@instructions = Array.new
		@startPos = 0
		@name = "Unknown"
		@authorName = "Anonymous"
		@fileName = nil
	end
	
	def [](x)
		@instructions[x]
	end

	def to_s
		s = sprintf("Program \"%s\" (length %d) by \"%s\"\n\n", @name, @instructions.length, @authorName);
		s += sprintf("%-6s %3s%3s  %6s\n", "", "ORG", "", "START");
		@instructions.each_index do |i|
			s += sprintf("%-6s %s\n", i == @startPos ? "START" : "", @instructions[i].to_s);
		end
		s
	end
	
	def levenstein(other, ins=1, del=1, sub=1)
		# ins, del, sub are weighted costs
		return nil if self.nil?
		return nil if other.nil?
		dm = [] #distance matrix
		
		# Initialize first row values
		dm[0] = (0..@instructions.length*6).collect { |i| i*ins }
		fill = [0]*(@instructions.length*6-1)
		
		# Initialize first column values
		for i in 1..other.instructions.length*6
			dm[i] = [i*del, fill.flatten]
		end

		#populate matrix
		for i in 1..other.instructions.length*6
			for j in 1..self.instructions.length*6
				# critical comparison
				dm[i][j] = [
					dm[i-1][j-1] + 
						(instructions[(j-1) / 6][(j-1) % 6] == other[(i-1)/6][(i-1) % 6] ? 0 : sub),
					dm[i][j-1] + ins,
					dm[i-1][j] + del
				].min
			end
		end
		
		# The last value in matrix is the Levenstein distance between the strings
		dm[other.instructions.length*6][self.instructions.length*6]
	end
	
end

# One Corewar instruction
class Instruction
	@@opcode = Converter.new [
		"DAT", "SPL", "MOV", "DJN", "ADD", "JMZ", 
		"SUB", "SEQ", "SNE", "SLT", "JMN", "JMP", 
		"NOP", "MUL", "MOD", "DIV", "LDP", "STP" 
		]
	@@opcodeModifier = Converter.new ["F", "A", "B", "AB", "BA", "X", "I" ]
	@@addressModifier = Converter.new [ "$", "#", "@", "<", ">", "*", "{", "}" ]
	
	attr_accessor :data
	
	def initialize(ins, modif, addra, numa, addrb, numb)
		@data = Array.new
		@data.push @@opcode[ins.upcase]
		@data.push @@opcodeModifier[modif.upcase]
		@data.push @@addressModifier[addra]
		@data.push numa
		@data.push @@addressModifier[addrb]
		@data.push numb
	end
	
	def [](x)
		@data[x]
	end
	
	def to_s
		sprintf("%3s.%-2s %s%6d, %s%6d %4s",
			@@opcode.decode(@data[0]),
			@@opcodeModifier.decode(@data[1]),
			@@addressModifier.decode(@data[2]),
			@data[3],
			@@addressModifier.decode(@data[4]),
			@data[5],
			"")
	end
end
