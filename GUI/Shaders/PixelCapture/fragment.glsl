layout(location = 0) in flat int outDrawId;

layout(location = 0) out vec4 FragColor;

void main(){

    int outDraw=outDrawId+1;
    FragColor.r = float((outDraw >> 16) & 0xFF) / 255.0;
    FragColor.g = float((outDraw >> 8)  & 0xFF) / 255.0;
    FragColor.b = float((outDraw)       & 0xFF) / 255.0;
    FragColor.a = 0;
}