begin
    GC.disable
    Start_Time = Time.now
    require_relative 'objects/ruby/loader_font'

rescue Exception => e
    puts e
    puts "---"
    puts e.backtrace
end
