#AngelCode BMFont loader written in reference to:
#  http://www.angelcode.com/products/bmfont/doc/file_format.html
#There is C code elsewhere that hooks to this ruby code and interprets the
#created dictionary.

def load_font_info(dict, line)
    dict["face"] = line.slice(/face=\"(?<grp>.+?)"/, "grp");
    dict["size"] = line.slice(/size=(?<grp>\d+)\s/, "grp").to_i;
    dict["bold"] = line.slice(/bold=(?<grp>\d+)\s/, "grp").to_i;
    dict["italic"] = line.slice(/italic=(?<grp>\d+)\s/, "grp").to_i;
    dict["charset"] = line.slice(/charset=\"(?<grp>.+?)"/, "grp");
    dict["unicode"] = line.slice(/unicode=(?<grp>\d+)\s/, "grp").to_i;
    dict["stretchH"] = line.slice(/stretchH=(?<grp>\d+)\s/, "grp").to_i;
    dict["smooth"] = line.slice(/smooth=(?<grp>\d+)\s/, "grp").to_i;
    dict["aa"] = line.slice(/aa=(?<grp>\d+)\s/, "grp").to_i;
    dict["padding"] = line.slice(/padding=(?<grp>.+?)\s/, "grp").split(",").map!(&:to_i);
    dict["spacing"] = line.slice(/spacing=(?<grp>.+?)\s/, "grp").split(",").map!(&:to_i);
    dict["outline"] = line.slice(/outline=(?<grp>\d+)\s/, "grp").to_i;
end

def load_font_common(dict, line)
    dict["lineHeight"] = line.slice(/lineHeight=(?<grp>\d+)\s/, "grp").to_i;
    dict["base"] = line.slice(/base=(?<grp>\d+)\s/, "grp").to_i;
    dict["scaleW"] = line.slice(/scaleW=(?<grp>\d+)\s/, "grp").to_i;
    dict["scaleH"] = line.slice(/scaleH=(?<grp>\d+)\s/, "grp").to_i;
    dict["pages"] = line.slice(/pages=(?<grp>\d+)\s/, "grp").to_i;
    dict["packed"] = line.slice(/packed=(?<grp>\d+)\s/, "grp").to_i;
    dict["alphaChnl"] = line.slice(/alphaChnl=(?<grp>\d+)\s/, "grp").to_i;
    dict["redChnl"] = line.slice(/redChnl=(?<grp>\d+)\s/, "grp").to_i;
    dict["greenChnl"] = line.slice(/greenChnl=(?<grp>\d+)\s/, "grp").to_i;
    dict["blueChnl"] = line.slice(/blueChnl=(?<grp>\d+)\s/, "grp").to_i;
end

def load_font_page(dict, line)
    id = line.slice(/id=(?<grp>\d+)\s/, "grp").to_i;
    dict[id] = line.slice(/file=\"(?<grp>.+?)"/, "grp");
end

def load_font_char(dict, line)
    id = line.slice(/id=(?<grp>\d+)\s/, "grp").to_i;
    dict[id] = {};
    dict[id]["id"] = id;
    dict[id]["x"] = line.slice(/x=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["y"] = line.slice(/y=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["width"] = line.slice(/width=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["height"] = line.slice(/height=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["xoffset"] = line.slice(/xoffset=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["yoffset"] = line.slice(/yoffset=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["xadvance"] = line.slice(/xadvance=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["page"] = line.slice(/page=(?<grp>\d+)\s/, "grp").to_i;
    dict[id]["chnl"] = line.slice(/chnl=(?<grp>\d+)\s/, "grp").to_i;
end

def load_font(path)
    dict = {};
    dict["info"] = {};
    dict["common"] = {};
    dict["page"] = {};
    dict["char"] = {};
    open(path).readlines.each do |line|
        load_font_info(dict["info"], line) if (line[0..3] == "info");
        load_font_common(dict["common"], line) if (line[0..5] == "common");
        load_font_page(dict["page"], line) if (line[0..3] == "page");
        load_font_char(dict["char"], line) if (line[0..3] == "char");
    end

    dict["char_ids"] = dict["char"].keys.sort();
    return dict;
end
