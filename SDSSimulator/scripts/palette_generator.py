

def hex_to_rgb(value):
    # code from http://stackoverflow.com/questions/214359/converting-hex-to-rgb-and-vice-versa
    # posted by Jeremy Cantrell
    value = value.lstrip('#')
    lv = len(value)
    return tuple(int(value[i:i+lv/3], 16) for i in range(0, lv, lv/3))

# palette generated from http://www.wellstyled.com/tools/colorscheme2/index-en.html
colours = '''#C7D9C3
#758073
#EBFFE6
#B0BFAC
#C3B0B8
#807378
#FFE6F1
#BFACB5
#D9D2C3
#807C73
#FFF7E6
#BFB9AC
#8B8B98
#757580
#E9E9FF
#AFAFBF'''

palette_in_hex = colours.split()

palette_in_rgb = [hex_to_rgb(hex) for hex in palette_in_hex]

# write palette to a C header
outfile = open('palette.h','w')
outfile.write('#ifndef ORGANISMPALETTE_H\n#define ORGANISM_PALETTE_H\n')
outfile.write('const int palette_size = %d;\n'%(len(palette_in_rgb)))
outfile.write('const int organism_palette[palette_size][3] = {') 
for rgb in palette_in_rgb:
    outfile.write('{%d,%d,%d},'%rgb) 
outfile.write('};\n#endif\n')