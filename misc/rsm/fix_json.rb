require 'strscan'

require 'json/pure'

# This allows to parse multiple objects from the same stream.

class Float

   def to_json(*) 
		return 'null' if nan?
		if not inf = infinite?
			return to_s
		else
			return to_s.to_json
		end
	end
end

module JSON
  module Pure

	class Parser
     def initialize(source, opts = {})
       super
       if !opts.key?(:max_nesting) # defaults to 19
         @max_nesting = 19
       elsif opts[:max_nesting]
         @max_nesting = opts[:max_nesting]
       else
         @max_nesting = 0
       end
       @create_id = JSON.create_id
       reset
     end


     # Parses the current JSON string _source_ and returns the complete data
     # structure as a result.
     def parse
       obj = nil
       until eos? || obj
         case
         when scan(OBJECT_OPEN)
           obj and raise ParserError, "source '#{peek(20)}' not in JSON!"
           @current_nesting = 1
           obj = parse_object
         when scan(ARRAY_OPEN)
           obj and raise ParserError, "source '#{peek(20)}' not in JSON!"
           @current_nesting = 1
           obj = parse_array
         when skip(IGNORE)
           ;
         else
           raise ParserError, "source '#{peek(20)}' not in JSON!"
         end
       end
       obj or raise ParserError, "source did not contain any JSON!"
       obj
     end

	end

  end
end

