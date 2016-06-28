#AngelCode BMFont loader written in reference to:
#  http://www.angelcode.com/products/bmfont/doc/file_format.html
#There is C code elsewhere that hooks to this ruby code and interprets the
#created dictionary.

def load_font_info(dict, line)
    dict["face"] = line.slice(/face=\"(?<grp>.+?)"/, "grp");
    dict["size"] = line.slice(/size=(?<grp>.+?)\s/, "grp").to_i;
    dict["bold"] = line.slice(/bold=(?<grp>.+?)\s/, "grp").to_i;
    dict["italic"] = line.slice(/italic=(?<grp>.+?)\s/, "grp").to_i;
    dict["charset"] = line.slice(/charset=\"(?<grp>.+?)"/, "grp");
    dict["unicode"] = line.slice(/unicode=(?<grp>.+?)\s/, "grp").to_i;
    dict["stretchH"] = line.slice(/stretchH=(?<grp>.+?)\s/, "grp").to_i;
    dict["smooth"] = line.slice(/smooth=(?<grp>.+?)\s/, "grp").to_i;
    dict["aa"] = line.slice(/aa=(?<grp>.+?)\s/, "grp").to_i;
    dict["padding"] = line.slice(/padding=(?<grp>.+?)\s/, "grp").split(",").map!(&:to_i);
    dict["spacing"] = line.slice(/spacing=(?<grp>.+?)\s/, "grp").split(",").map!(&:to_i);
    dict["outline"] = line.slice(/outline=(?<grp>.+?)\s/, "grp").to_i;
end

def load_font_common(dict, line)
    dict["lineHeight"] = line.slice(/lineHeight=(?<grp>.+?)\s/, "grp").to_i;
    dict["base"] = line.slice(/base=(?<grp>.+?)\s/, "grp").to_i;
    dict["scaleW"] = line.slice(/scaleW=(?<grp>.+?)\s/, "grp").to_i;
    dict["scaleH"] = line.slice(/scaleH=(?<grp>.+?)\s/, "grp").to_i;
    dict["pages"] = line.slice(/pages=(?<grp>.+?)\s/, "grp").to_i;
    dict["packed"] = line.slice(/packed=(?<grp>.+?)\s/, "grp").to_i;
    dict["alphaChnl"] = line.slice(/alphaChnl=(?<grp>.+?)\s/, "grp").to_i;
    dict["redChnl"] = line.slice(/redChnl=(?<grp>.+?)\s/, "grp").to_i;
    dict["greenChnl"] = line.slice(/greenChnl=(?<grp>.+?)\s/, "grp").to_i;
    dict["blueChnl"] = line.slice(/blueChnl=(?<grp>.+?)\s/, "grp").to_i;
end

def load_font_page(dict, line)
    id = line.slice(/id=(?<grp>.+?)\s/, "grp").to_i;
    dict[id] = line.slice(/file=\"(?<grp>.+?)"/, "grp");
end

def load_font_char(dict, line)
    char = {};
    #grab the char id and convert it to an actual Unicode char
    char["id"] = line.slice(/id=(?<grp>.+?)\s/, "grp").to_i;
    #grab the rest of the information
    char["x"] = line.slice(/x=(?<grp>.+?)\s/, "grp").to_i;
    char["y"] = line.slice(/y=(?<grp>.+?)\s/, "grp").to_i;
    char["width"] = line.slice(/width=(?<grp>.+?)\s/, "grp").to_i;
    char["height"] = line.slice(/height=(?<grp>.+?)\s/, "grp").to_i;
    char["xoffset"] = line.slice(/xoffset=(?<grp>.+?)\s/, "grp").to_i;
    char["yoffset"] = line.slice(/yoffset=(?<grp>.+?)\s/, "grp").to_i;
    char["xadvance"] = line.slice(/xadvance=(?<grp>.+?)\s/, "grp").to_i;
    char["page"] = line.slice(/page=(?<grp>.+?)\s/, "grp").to_i;
    char["chnl"] = line.slice(/chnl=(?<grp>.+?)\s/, "grp").to_i;
    dict["chars"] << char;
end

def load_font(path)
    dict = {};
    dict["info"] = {};
    dict["common"] = {};
    dict["page"] = {};
    dict["chars"] = [];

    open(path).readlines.each do |line|
        load_font_info(dict["info"], line) if (line[0..4] == "info ");
        load_font_common(dict["common"], line) if (line[0..6] == "common ");
        load_font_page(dict["page"], line) if (line[0..4] == "page ");
        load_font_char(dict, line) if (line[0..4] == "char ");
    end

    return dict;
end
