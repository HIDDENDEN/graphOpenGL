
//ФРАГМЕНТНЫЙ ШЕЙДЕР - расчет для фрагментов объекта, а вершинный шейдер - расчет для вершин (вершин обычно гораздо меньше чем фпагментов)

#version 330 core
struct Material
{
    //vec3 ambient;// фоновое освещение обычно совпадает с диффузным
    sampler2D diffuse;//рассеивание света(чем ближе к источнику - тем ярче)
    sampler2D specular;//блик
    sampler2D emission;//эмиссия
    /*
     карта эмиссии хранит значение свечения каждого фрагмента объекта. Значения эмиссии — это цвета, которые объект может излучать, как если бы он содержал источник света.
     */
    float shininess;//коэф-т силы блика
};
struct Light {
    vec3 position;// фоновое освещение
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


  
out vec4 FragColor;

in vec3 Normal;//принимаем вектор нормали
in vec3 FragPos;//принимаем позицию фрагмента вершины объекта
in vec2 TexCoords;//координаты текстуры

//uniform vec3 objectColor;
//uniform vec3 lightColor;
//uniform vec3 lightPos;
uniform vec3 viewPos;//позиция наблюдателя(нужно для создания бликов)
uniform Material material;
uniform Light light;
uniform float time;

void main()
{
    //ambient - фоновое освещение
    //vec3 ambient  = light.ambient * material.ambient;
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    // (совпадает с диффузным освещением)
    
    //diffuse - рассеивание света(чем ближе к источнику - тем ярче)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);//вектор направления освещения от источника к фрагменту вершины
    
    float diff = max(dot(norm, lightDir), 0.0);//скалярным произведение получаем угол между вектором нормали и вектором освещенности
    //чем меньше угол(больше скалярное произв.) тем сильнее диффузное свечение
    
    //цвет от "диффузии"
    //делаем выборку из текстуры, чтобы извлечь значение диффузного цвета фрагмента
    vec3 diffuse = light.diffuse *diff * vec3(texture(material.diffuse,                                                                     TexCoords));
    
    //specular - блик(блеск)
    vec3 viewDir = normalize(viewPos - FragPos);//вектор направления взгляда
    vec3 reflectDir = reflect(-lightDir, norm);//вектор направления блика
    /*
     вектор lightDir в настоящее время указывает в обратную сторону, то есть от фрагмента к источнику света (направление зависит от порядка вычитания векторов, которое мы делали при вычислении вектора lightDir). Поэтому, для получения правильного вектора отражения, мы меняем его направление на противоположное посредством инверсии вектора lightDir
     */
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //pow - возведение в степень
    
    vec3 specular = light.specular * spec *
                                vec3(texture(material.specular,TexCoords));
    
    //vec3 specular = light.specular * spec *
    //(vec3(1.0f)- vec3(texture(material.specular,TexCoords)));//этим вычитанием мы инвертируем бликовую карту: черное становится белым, белое - чёрным
    /*
        Сначала вычисляется скалярное произведение векторов отражения и направления взгляда (с отсевом отрицательных значений), а затем результат возводится в 32-ю степень. Константное значение 32 задает силу блеска. Чем больше это значение, тем сильнее свет будет отражаться, а не рассеиваться, и тем меньше станет размер пятна блика
    */
    
    /*Emission */
        vec3 emission = vec3(0.0);
        if (texture(material.specular, TexCoords).r == 0.0)   /*rough check for blackbox inside spec texture */
        {
            /*apply emission texture */
            emission = texture(material.emission, TexCoords).rgb;
            
            /*some extra fun stuff with "time uniform" */
            emission = texture(material.emission, TexCoords + vec2(0.0,time)).rgb;   /*moving */
            emission = emission * (cos(time) * 0.5 + 0.5) * 0.8;                     /*fading */
        }
        
       
    
    
    FragColor =vec4(ambient + diffuse + specular + emission, 1.0f);
    
}
