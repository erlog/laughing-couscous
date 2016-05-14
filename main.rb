begin
    require_relative 'wavefront'

    GC.disable
    Start_Time = Time.now

    def log(string)
        elapsed = (Time.now - Start_Time).round(3)
        puts "#{elapsed}: #{string}"
    end

    def load_object(object_name)
        object = Wavefront.from_file("objects/#{object_name}/object.obj")
        return object.faces
    end

    def ruby_update()
        puts("hello");
        #objects.each do |object|
        #    render_model(object[0], view_matrix, normal_matrix,
        #        camera_direction, light_direction,
        #        bitmap, z_buffer, object[1], object[2], object[3])
        #end
    end
rescue Exception => e
    puts e
    puts "---"
    puts e.backtrace
end
