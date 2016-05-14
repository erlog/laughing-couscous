require 'matrix'

class Point
    def self.from_array(array)
        return Point.new(array[0], array[1], array[2]);
        return self;
    end

    def u
        return self.x;
    end

    def v
        return self.y;
    end

    def +(other)
        return Point.new(self.x + other.x, self.y + other.y, self.z + other.z)
    end

    def -(other)
        return Point.new(self.x - other.x, self.y - other.y, self.z - other.z)
    end
end

def compute_face_tb(face) #tangent/bitangent
    a, b, c = face
    q1 = b.v - a.v; q2 = c.v - a.v
    s1t1 = b.uv - a.uv; s2t2 = c.uv - a.uv
    if s1t1 == s2t2 #otherwise we get NaN trying to divide infinity
        s1t1 = Point.new(1.0, 0.0, 0.0)
        s2t2 = Point.new(0.0, 1.0, 0.0)
    end
    st_matrix = Matrix[ [s2t2.v, -1*s1t1.v],
                        [-1*s2t2.u, s1t1.u] ]
    q_matrix = Matrix[ [q1.x, q1.y, q1.z],
                       [q2.x, q2.y, q2.z] ]
    tb_matrix = (st_matrix * q_matrix)/(s1t1.u*s2t2.v - s1t1.v*s2t2.u)
    t = Point.from_array(tb_matrix.row(0)).normalize
    b = Point.from_array(tb_matrix.row(1)).normalize
    return [t,b]
end

class Indexed_Vertex
    #Shim class to load indexed vertex information in before processing and
    #sending to C_Vertex which requires proper Point* objects instead of ints
    attr_accessor :v; attr_accessor :uv; attr_accessor :normal;
    def initialize(geometric_vertex, texture_coordinate, normal)
        @v = geometric_vertex; @uv = texture_coordinate; @normal = normal;
    end
end

class Wavefront
#TODO: no reason for this to be a class anymore, should create a Face struct in
#C and just return a Ruby array of C_Faces that gets converted on load
    attr_reader :faces

    def initialize(faces, vertices, uvs, normals)
        @vertices = vertices
        @uvs = uvs
        @normals = normals
        @tangents = Array.new(vertices.length){[]}
        @bitangents = Array.new(vertices.length){[]}

        #compute tangents/bitangents for tangent space normal mapping
        for indexed_face in faces
            vertex_indices = indexed_face.map{ |vertex| vertex.v }
            face = indexed_face.map{ |vertex|
                    vertex = vertex.dup
                    vertex.v = @vertices[vertex.v]
                    vertex.uv = @uvs[vertex.uv]
                    vertex.normal = @normals[vertex.normal]
                    vertex
                    }
            tangent, bitangent = compute_face_tb(face)
            for index in vertex_indices
                @tangents[index] << tangent
                @bitangents[index] << bitangent
            end
        end
        @faces = faces

        #average face tangent/bitangets to get t/b at individual vertices
        @tangents.map!{|group| group.inject(&:+).scale_by_factor(1.0/group.length) }
        @bitangents.map!{|group| group.inject(&:+).scale_by_factor(1.0/group.length) }

        #Send everything over to C
        @faces.map!{ |indexed_face| self.build_face(indexed_face) }
        return @faces
    end

    def build_face(indexed_face)
        return C_Face.new( self.build_vertex(indexed_face[0]),
            self.build_vertex(indexed_face[1]), self.build_vertex(indexed_face[2]))
    end

    def build_vertex(vertex)
        v = @vertices[vertex.v]
        uv = @uvs[vertex.uv]
        normal = @normals[vertex.normal]
        tangent = @tangents[vertex.v]
        bitangent = @bitangents[vertex.v]
        return C_Vertex.new(v, uv, normal, tangent, bitangent)
    end

	def self.from_file(file_path)
        #log("Loading model: #{file_path}")
        faces, vertices, uvs, normals  = [], [], [], []

        lines = open(file_path).readlines.map!(&:strip)
        for line in lines
            parts = line.split(" ")
            if parts[0] == "v"
                x, y, z = parts[1..-1].map(&:to_f)
                vertices << Point.new(x, y, z)

            elsif parts[0] == "vt"
                x, y, z = parts[1..-1].map(&:to_f)
                uvs << Point.new(x, y, z)

            elsif parts[0] == "vn"
                x, y, z = parts[1..-1].map(&:to_f)
                normals << Point.new(x, y, z)

            elsif parts[0] == "f"
                v, vt, vn = parts[1].split("/").map{ |index| index.to_i - 1}
                a = Indexed_Vertex.new(v, vt, vn)
                v, vt, vn = parts[2].split("/").map{ |index| index.to_i - 1}
                b = Indexed_Vertex.new(v, vt, vn)
                v, vt, vn = parts[3].split("/").map{ |index| index.to_i - 1}
                c = Indexed_Vertex.new(v, vt, vn)
                faces << [a, b, c]
            end

        end
        return Wavefront.new(faces, vertices, uvs, normals)
    end
end

def face_to_screen(face, view_matrix, screen_center)
    face = face.dup
    face[0].v = face[0].v.dup.apply_matrix!(view_matrix).to_screen!(screen_center)
    face[1].v = face[1].v.dup.apply_matrix!(view_matrix).to_screen!(screen_center)
    face[2].v = face[2].v.dup.apply_matrix!(view_matrix).to_screen!(screen_center)
    face = face.sort_by(&:v)
    return face
end

def compute_face_normal(face)
    a, b, c = face
    return ((b.v - a.v).cross_product(c.v - a.v)).normalize!
end

