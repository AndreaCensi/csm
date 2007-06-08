require 'strscan'

require 'json/pure'

# This allows to parse multiple objects from the same stream.

module JSON
  module Pure

    class Parser 
      # Creates a new JSON::Pure::Parser instance for the string _source_.
      def initialize(source)
        super
        @create_id = JSON.create_id
        reset # added
      end

      # Parses the current JSON string _source_ and returns the complete data
      # structure as a result.
      def parse
#        reset
        obj = nil
        until eos? || obj    # added obj
          case
          when scan(OBJECT_OPEN)
            obj and raise ParserError, "source '#{peek(20)}' not in JSON!"
            obj = parse_object
          when scan(ARRAY_OPEN)
            obj and raise ParserError, "source '#{peek(20)}' not in JSON!"
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
