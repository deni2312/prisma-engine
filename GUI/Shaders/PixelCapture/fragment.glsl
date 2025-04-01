layout(location = 0) in flat int outDrawId;

layout(location = 0) out vec4 FragColor;

void main(){
    FragColor.r = floor(float((outDrawId >> 16) & 0xFF)) / 255.0;
    FragColor.g = floor(float((outDrawId >> 8)  & 0xFF)) / 255.0;
    FragColor.b = floor(float((outDrawId)       & 0xFF)) / 255.0;
    FragColor.a = 1.0;
}