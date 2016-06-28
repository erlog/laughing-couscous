begin
    GC.disable;
    #TODO: get rid of this in favor of a Ruby global function that
    #can interface with our state object
    Start_Time = Time.now;
    require_relative 'loader_font'

    def load_strings(path)
        string_dicts = []; id = 0;

        open(path).readlines.each do |line|
            if line != ""
                dict = {};
                dict["id"] = id;
                dict["string"] = line.rstrip;
                string_dicts << dict;
            end
            id += 1;
        end

        return string_dicts;
    end

rescue Exception => e
    puts e;
end
