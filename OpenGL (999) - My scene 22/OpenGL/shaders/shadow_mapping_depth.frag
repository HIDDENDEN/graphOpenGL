
//ФРАГМЕНТНЫЙ ШЕЙДЕР - расчет для фрагментов объекта, а вершинный шейдер - расчет для вершин (вершин обычно гораздо меньше чем фпагментов).
//Так как у нас нет цветового буфера в буфере кадров для теней, фрагментный шейдер не требует никаких вычислений, и мы может оставить его пустым
//и в конце работы шейдера мы получим обновлённый буфер глубины
#version 330 core

void main()
{
    // gl_FragDepth = gl_FragCoord.z;
}


