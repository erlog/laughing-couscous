begin
    #TODO: get rid of this in favor of a Ruby global function that
    #can interface with our state object
    require_relative 'loader_font'

    def unique_chars(string_data)
        #TODO:I could use a set here, but I need to research how standard
        #library imports might affect dependencies
        uniques = [];
        string_data.split("").each do |char|
            char = char.strip;
            next if char == "";
            uniques << char if !uniques.include?(char);
        end
        uniques.sort!
        return uniques;
    end

    def load_strings(path)
        string_dicts = []; id = 0;
        lines = open(path).readlines;

        #write unique chars
        #open(path+"_uniques", "w").write(unique_chars(open(path).read).join(""));

        lines.each do |line|
            if line != ""
                dict = {}; dict["id"] = id; dict["string"] = line.rstrip;
                string_dicts << dict;
            end
            id += 1;
        end

        return string_dicts;
    end

    def load_ini(path)
        dict = {};

        open(path).readlines.each do |line|
            line = line.strip;
            next if line[0] == "#";
            line = line.split("=");
            dict[line[0]] = line[1].to_i;
        end

        return dict;
    end

rescue Exception => e
    puts e;
    puts "---";
    puts e.backtrace;
end
