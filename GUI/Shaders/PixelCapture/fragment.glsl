layout(location = 3) in flat int outDrawId;

layout(location = 0) out vec4 FragColor;

void main(){
	// Extract the RGB components from the UUID
    float r = float((outDrawId >> 16) & 0xFF) / 255.0;
    float g = float((outDrawId >> 8) & 0xFF) / 255.0;
    float b = float(outDrawId & 0xFF) / 255.0;

    // Assign the RGB components to the fragment color, alpha is set to 1.0
    FragColor = vec4(r, g, b, 0.0);

}