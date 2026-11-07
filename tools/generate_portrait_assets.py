#!/usr/bin/env python3
"""Rasterize native-size portrait fonts and antialiased lane masks.

The generated C++ is checked in; normal firmware builds do not need Pillow
or a system font. Supply a licensed TrueType font with --font to regenerate.
"""
import argparse
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / "waze-hud/main/assets"
TEXT = sorted(set(range(32, 127)) | set(range(0xC0, 0x180)) |
              set(range(0x1EA0, 0x1EFA)) | {0x1A0, 0x1A1, 0x1AF, 0x1B0})
DIGITS = list(map(ord, "-0123456789?"))

def bytes_cpp(data):
    return ",\n".join("    " + ",".join(f"0x{b:02x}" for b in data[i:i+24]) for i in range(0,len(data),24)) or "0"

def font_cpp(name, path, size, codes):
    font = ImageFont.truetype(str(path), size)
    boxes = [font.getbbox(chr(c), anchor="la") for c in codes]
    top, bottom = min(b[1] for b in boxes), max(b[3] for b in boxes)
    data, glyphs = bytearray(), []
    for code, (l,t,r,b) in zip(codes, boxes):
        w,h = r-l,b-t
        glyphs.append(f"{{{code},{len(data)},{w},{h},{l},{t-top},{round(font.getlength(chr(code)))}}}")
        if w and h:
            image=Image.new("L",(w,h)); ImageDraw.Draw(image).text((-l,-t),chr(code),font=font,fill=255,anchor="la")
            pixels=list(image.get_flattened_data())
            data.extend(((pixels[i]>>4)<<4) | (pixels[i+1]>>4 if i+1<len(pixels) else 0) for i in range(0,len(pixels),2))
    return (f"static const uint8_t {name}Bitmap[]={{\n{bytes_cpp(data)}\n}};\n"+
            f"static const FontGlyph {name}Glyphs[]={{"+",\n".join(glyphs)+"};\n"+
            f"const BitmapFont {name}{{{name}Glyphs,{len(glyphs)},{name}Bitmap,{bottom-top}}};\n")

def lane_mask(bit):
    scale=4
    image=Image.new("L",(36*scale,36*scale)); d=ImageDraw.Draw(image)
    def stroke(points,width=3):
        points=[(round(x*scale),round(y*scale)) for x,y in points]
        d.line(points,fill=255,width=width*scale,joint="curve")
        rad=width*scale/2
        for x,y in points:
            d.ellipse((round(x-rad),round(y-rad),round(x+rad),round(y+rad)),fill=255)
    def curve(p0,p1,p2):
        return [((1-t)**2*p0[0]+2*(1-t)*t*p1[0]+t*t*p2[0],
                 (1-t)**2*p0[1]+2*(1-t)*t*p1[1]+t*t*p2[1]) for t in (i/16 for i in range(17))]
    if bit==-1: stroke([(18,32),(18,20)])
    elif bit==0: stroke([(18,22),(18,5)]);stroke([(12,11),(18,5),(24,11)])
    elif bit in (1,4):
        pts=curve((18,22),(18,17),(30,6)); head=[(22,6),(30,6),(30,14)]
        if bit==1: pts=[(36-x,y) for x,y in pts];head=[(36-x,y) for x,y in head]
        stroke(pts);stroke(head)
    elif bit in (2,3,5,6):
        pts=curve((18,22),(18,11),(9,11))+[(4,11)];head=[(10,5),(4,11),(10,17)]
        if bit in (5,6):pts=[(36-x,y) for x,y in pts];head=[(36-x,y) for x,y in head]
        stroke(pts);stroke(head)
    else:
        stroke(curve((18,22),(18,5),(10,5))+curve((10,5),(4,5),(4,13))+[(4,21)])
        stroke([(1,16),(4,21),(9,16)])
    return bytes(image.resize((36,36),Image.Resampling.LANCZOS).get_flattened_data())

def main():
    p=argparse.ArgumentParser();p.add_argument("--font",type=Path,required=True);args=p.parse_args()
    fonts=[("kHudTiny",10,TEXT),("kHudSmall",12,TEXT),("kHudHeader",16,TEXT),
           ("kHudSpeed",64,DIGITS),("kHudSpeed3",52,DIGITS),("kHudLimit",32,DIGITS),
           ("kHudLimit3",24,DIGITS),("kHudExit",24,DIGITS)]
    header='#pragma once\n#include "assets/generated_assets.h"\nnamespace waze_hud::assets {\n'
    cpp='#include "assets/portrait_assets.h"\nnamespace waze_hud::assets {\n'
    for name,size,codes in fonts:
        header+=f"extern const BitmapFont {name};\n";cpp+=font_cpp(name,args.font,size,codes)
    for i in range(-1,8):cpp+=f"static const uint8_t lane{i+1}[]={{\n{bytes_cpp(lane_mask(i))}\n}};\n"
    header+='extern const AlphaMask kPortraitLaneStem;\nextern const AlphaMask kPortraitLaneBranches[8];\n}\n'
    cpp+='const AlphaMask kPortraitLaneStem{36,36,lane0};\nconst AlphaMask kPortraitLaneBranches[8]={'+','.join(f'{{36,36,lane{i+1}}}' for i in range(8))+'};\n}\n'
    (DEST/'portrait_assets.h').write_text(header)
    (DEST/'portrait_assets.cpp').write_text(cpp)
    print('Generated native fonts and lane masks')
if __name__=='__main__': main()
