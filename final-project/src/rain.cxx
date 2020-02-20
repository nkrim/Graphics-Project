#include "rain.hxx"




void Rain::InitCoords(View* v) {
    int index = 0;
    float rand_val = 0.0f;


    for (int i = NUM_GRASS; i < NUM_DROPS+NUM_GRASS; i++) {
        this->indices[i-NUM_GRASS] = i-NUM_GRASS;

        this->verts[i-NUM_GRASS] = cs237::vec3f(0.0f,0.0f,0.0f);

    }








    glBindBuffer(GL_TEXTURE_BUFFER, v->_texBufObj);
    void *ptr = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
    cs237::vec4f* c = (cs237::vec4f*) ptr;
    for (int i = NUM_GRASS; i < NUM_DROPS+NUM_GRASS; i++) {
        c[i] = cs237::vec4f(
            v->Camera().position()[0]  + 200.0f* ( (rand() % 100) / 100.0f ) * cos( (2.0f * M_PI) * (rand() % 100) / 100.0f ),
            (rand() % 200),
            v->Camera().position()[2]  + 200.0f* ( (rand() % 100) / 100.0f ) * sin( (2.0f * M_PI) * (rand() % 100) / 100.0f ),
            0.0f);

    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    c = (cs237::vec4f*) (ptr = NULL);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);



}

// TODO store sins and thetas
void Rain::Update(View* v) {
    glBindBuffer(GL_TEXTURE_BUFFER, v->_texBufObj);
    void *ptr = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
    cs237::vec4f* c = (cs237::vec4f*) ptr;
    for (int i = NUM_GRASS; i < NUM_DROPS+NUM_GRASS; i++) {
        c[i][3] -= 0.2f;

        c[i][1] += c[i][3];

        if (c[i][1] < -20.0f) {
            c[i][1] = v->Camera().position()[1] + rand()%100;
            c[i][3] = 0.0f;


            c[i][0] = v->Camera().position()[0] + 200.0f * ( (rand() % 100) / 100.0f ) * cos( (2.0f * M_PI) * (rand() % 100) / 100.0f );
            c[i][2] = v->Camera().position()[2] + 200.0f * ( (rand() % 100) / 100.0f ) * sin( (2.0f * M_PI) * (rand() % 100) / 100.0f );
        }
        // printf("%f, %f, %f\n", c[i][0], c[i][1], c[i][2]);
    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    c = (cs237::vec4f*) (ptr = NULL);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}



void Rain::Init(View* v) {





    CS237_CHECK( glGenVertexArrays (1, &this->_id) );

    GLuint buf[2];
    CS237_CHECK( glGenBuffers (2, buf) );

    this->_vBuf = buf[0];
    this->_iBuf = buf[1];

    this->_nIndices = 0;


    cs237::image2d *img = new cs237::image2d("../data/raindropcopy.png");
    texture = new cs237::texture2D(GL_TEXTURE_2D, img);

    texture->Parameter (GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    texture->Parameter (GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

void Rain::Load() {



    CS237_CHECK( glBindVertexArray (this->_id) );

    // setup the vertex array (3 floats per vertex)
    CS237_CHECK( glBindBuffer (GL_ARRAY_BUFFER, this->_vBuf) );
    CS237_CHECK( glBufferData (GL_ARRAY_BUFFER, NUM_DROPS * sizeof(cs237::vec3f), verts, GL_STATIC_DRAW) );

    CS237_CHECK( glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, 0) );
    CS237_CHECK( glEnableVertexAttribArray (0) );

    // setup index array
    this->_nIndices = NUM_DROPS;
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->_iBuf) );
    CS237_CHECK( glBufferData (GL_ELEMENT_ARRAY_BUFFER, NUM_DROPS * sizeof(uint32_t), this->indices, GL_STATIC_DRAW) );






    CS237_CHECK( glBindBuffer (GL_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindVertexArray (0) );

}

void Rain::Draw() {
    // CS237_CHECK( glBindBuffer(GL_TEXTURE_BUFFER, _texBufObj) );
    // CS237_CHECK( glBindTexture(GL_TEXTURE_BUFFER, _texBuf) );


    CS237_CHECK(glActiveTexture(GL_TEXTURE3));
    texture->Bind();

    CS237_CHECK( glBindVertexArray (this->_id) );
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->_iBuf) );



    CS237_CHECK( glDrawArrays (GL_POINTS, 0, this->_nIndices) );




    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindVertexArray (0) );

}


