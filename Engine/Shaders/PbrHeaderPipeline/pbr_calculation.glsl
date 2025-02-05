vec3 pbrCalculation(vec3 FragPos, vec3 N, vec3 albedo, vec4 aoSpecular,float roughness,float metallic) {

    float specularMap = aoSpecular.r;
    float ao = aoSpecular.g;

    vec3 V = normalize(viewPos.xyz - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 R = reflect(-V, N);

    for (int i = 0; i < lenDir.r; i++) {

        vec3 L = normalize(vec3(directionalData[i].direction));
        vec3 H = normalize(V + L);


        vec3 radiance = vec3(directionalData[i].diffuse);


        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F * specularMap;

        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        if (directionalData[i].padding.x < 1.0) {
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
        else {
            Lo += (kD * albedo / PI + specular) * radiance * NdotL * (1 - ShadowCalculationDirectional(FragPos, L, N, i));
        }
    }

    float dotNV = clamp(dot(N, V), 0.0f, 1.0f);

    // use roughness and sqrt(1-cos_theta) to sample M_texture
    vec2 uv = vec2(roughness, sqrt(1.0f - dotNV));
    uv = uv*LUT_SCALE + LUT_BIAS;

    // get 4 parameters for inverse_M
    vec4 t1 = texture(textureM, uv);

    // Get 2 parameters for Fresnel calculation
    vec4 t2 = texture(textureLut, uv);

    mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(  0,  1,    0),
        vec3(t1.z, 0, t1.w)
    );
    vec3 mSpecular = ToLinear(vec3(0.23f, 0.23f, 0.23f)); // mDiffuse
    for (int i = 0; i < lenArea.r; i++) {
        		// Evaluate LTC shading

        vec3 position[4];
        position[0]=vec3(areaData[i].position[0]);
        position[1]=vec3(areaData[i].position[1]);
        position[2]=vec3(areaData[i].position[2]);
        position[3]=vec3(areaData[i].position[3]);


		vec3 diffuse = LTC_Evaluate(N, V, FragPos, mat3(1), position, false);
		vec3 specular = LTC_Evaluate(N, V, FragPos, Minv, position, false);

		// GGX BRDF shadowing and Fresnel
		// t2.x: shadowedF90 (F90 normally it should be 1.0)
		// t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
		specular *= mSpecular*t2.x + (1.0f - mSpecular) * t2.y;

		// Add contribution
		Lo += vec3(areaData[i].diffuse) * (specular + albedo * diffuse);
    }

    // Locating which cluster this fragment is part of
    uint zTile = uint((log(abs(vec3(view * vec4(FragPos, 1.0)).z) / zNear) * gridSize.z) / log(zFar / zNear));
    vec2 tileSize = screenDimensions.xy / gridSize.xy;
    uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
    uint tileIndex =
        tile.x + (tile.y * gridSize.x) + (tile.z * gridSize.x * gridSize.y);

    uint lightCount = clusters[tileIndex].count;

    for (int i = 0; i < lightCount; i++) {
        uint lightIndex = clusters[tileIndex].lightIndices[i];

        vec3 distance = vec3(omniData[lightIndex].position) - FragPos;
        vec3 L = normalize(distance);
        vec3 H = normalize(V + L);

        float totalDistance = length(distance);

        if (totalDistance <= omniData[lightIndex].radius) {

            vec3 radiance = vec3(omniData[lightIndex].diffuse);


            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F * specularMap;

            vec3 kD = vec3(1.0) - kS;

            kD *= 1.0 - metallic;

            float NdotL = max(dot(N, L), 0.0);

            float attenuation = 1.0 / (omniData[lightIndex].attenuation.x + omniData[lightIndex].attenuation.y * totalDistance + omniData[lightIndex].attenuation.z * totalDistance * totalDistance);

            if (omniData[lightIndex].padding.x < 1.0) {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation;
            }
            else {
                Lo += (kD * albedo / PI + specular) * radiance * NdotL * attenuation * (1 - ShadowCalculation(FragPos, vec3(omniData[lightIndex].position), int(lightIndex)));
            }

        }
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F * specularMap;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    Lo = ambient + Lo;
    return Lo;
}