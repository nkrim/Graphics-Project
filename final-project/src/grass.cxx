#include "grass.hxx"


void Grass::Load() {
    // load all sides
    for (int i = 0; i < 3; i++) {
        sides[i].Load();
    }
}

// TODO: frustum cull grass and rain.
void Grass::InitCoords(View* v) {

    int index = 0;

    for (int i = 0; i < NUM_GRASS; i++) {
        randoms[i][0] = (float) ((rand() % 100) - 50) / 20.0f;
        randoms[i][1] = (float) ((rand() % 100) - 50) / 20.0f;
    }

    int dim = sqrt(NUM_GRASS);
    for(float y = -dim/2; y < dim/2; y ++) {
        for(float x = -dim/2; x < dim/2; x ++) {


            // translations[index][0] = v->Camera().position()[0] + (x * 7.0f) + 10.0 * randoms[index][0] + 3.232532f;

            // translations[index][2] = v->Camera().position()[2] + (y * 10.0f) + 10.0 * randoms[index][1] + 3.235435f;

            // translations[index][1] = v->FindHeightAt(cs237::vec3d(
            //     translations[index][0],0.0f,translations[index][2]
            // ));


            // random floats to avoid it coinciding with edge of triangle in mesh
            translations[index][0] = v->Camera().position()[0] + (x * 7.0f) + 2.0 * randoms[index][0]+ 3.232532f;
            translations[index][2] = v->Camera().position()[0] + (y * 7.0f) + 2.0 * randoms[index][1]+ 3.235435f;
            translations[index][1] = v->FindHeightAt(cs237::vec3d(
                translations[index][0],0.0f,translations[index][2]
            ));

            // printf("%f\n", translations[index][1]);

            index++;
        }
    }




    cs237::vec3f* quadVerts[6] = {
        gquadVertsFront,
        gquadVertsLeft,
        gquadVertsRight
    };

    for (int i = 0; i < 3; i++) {
        sides[i].Copy(
                quadVerts[i],
                gquadIndices,
                gtextCoords
            );

        sides[i].InitCoords(v);

        // for (int j = 0; j < 4; j++) {
        //     sides[i].verts[j] += d_to_f(cam.position());
        // }
    }





}

void Grass::Draw() {
    // draw all sides
    for (int i = 0; i < 3; i++) {
        sides[i].Draw();
    }

}

void Grass::Init(View* v) {




    for (int i = 0; i < 3; i++) {
        sides[i].Init(v);
        cs237::image2d *img = new cs237::image2d("../data/grass7.png");
        sides[i].texture = new cs237::texture2D(GL_TEXTURE_2D, img);

        sides[i].texture->Parameter (GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        sides[i].texture->Parameter (GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }


}

float dist(cs237::vec3f campos, float x, float z) {
    return sqrt( pow(x - campos[0], 2) + pow(z - campos[2], 2));
}


void GQuad::Update(View* v) {
    glBindBuffer(GL_TEXTURE_BUFFER, v->_texBufObj);
    void *ptr = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
    cs237::vec4f* c = (cs237::vec4f*) ptr;
    for (int i = 0; i < NUM_GRASS; i++) {
        if ( dist(d_to_f(v->Camera().position()), c[i][0], c[i][2]) > 200.0f ) {
            // printf("%d\n", i);



            // TODO: fix this in a few years
            // c[i][0] = v->Camera().position()[0] + (v->Camera().position()[0] - c[i][0]) * 0.8;

            // c[i][2] = v->Camera().position()[2] + (v->Camera().position()[2] - c[i][2]) * 0.8;

            // c[i][1] = v->FindHeightAt(cs237::vec3d(
            //     c[i][0],0.0f,c[i][2]
            // ));




        }



    }

    // printf("%f\n", c[0][1]);
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    c = (cs237::vec4f*) (ptr = NULL);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void Grass::Update(View* v) {
    for(int i = 0 ; i < 3; i++) {
        sides[i].Update(v);
    }
}


void GQuad::InitCoords(View* v) {

    glBindBuffer(GL_TEXTURE_BUFFER, v->_texBufObj);
    void *ptr = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
    cs237::vec4f* c = (cs237::vec4f*) ptr;
    for (int i = 0; i < NUM_GRASS; i++) {

        c[i][0] = translations[i][0];

        c[i][2] = translations[i][2];

        c[i][1] = translations[i][1];
        // printf("%f\n", c[i][1]);




    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    c = (cs237::vec4f*) (ptr = NULL);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);


}



void GQuad::Copy(cs237::vec3f* verts, uint32_t* indices, cs237::vec2f* texts) {

    for (int i = 0; i < 4; i++) {
        this->verts[i] = verts[i];
        this->indices[i] = indices[i];
        this->text[i] = texts[i];

    }

    for (int i = 4; i < 6; i++) {
        this->indices[i] = indices[i];
    }

}

void GQuad::Init(View* v) {







    CS237_CHECK( glGenVertexArrays (1, &this->_id) );

    GLuint buf[4];
    CS237_CHECK( glGenBuffers (4, buf) );

    this->_vBuf = buf[0];
    this->_iBuf = buf[1];
    this->_tBuf = buf[2];
    this->_insBuf = buf[3];
    this->_nIndices = 0;
}

void GQuad::Load() {



    CS237_CHECK( glBindVertexArray (this->_id) );

    // setup the vertex array (3 floats per vertex)
    CS237_CHECK( glBindBuffer (GL_ARRAY_BUFFER, this->_vBuf) );
    CS237_CHECK( glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(cs237::vec3f), verts, GL_STATIC_DRAW) );

    CS237_CHECK( glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, 0) );
    CS237_CHECK( glEnableVertexAttribArray (0) );

    // setup index array
    this->_nIndices = 6;
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->_iBuf) );
    CS237_CHECK( glBufferData (GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t), this->indices, GL_STATIC_DRAW) );


    // setup textures
    CS237_CHECK( glBindBuffer (GL_ARRAY_BUFFER, this->_tBuf) );
    CS237_CHECK( glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(cs237::vec2f), text, GL_STATIC_DRAW) );

    CS237_CHECK( glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, 0) );
    CS237_CHECK( glEnableVertexAttribArray (1) );

    // add array for instancing?

    CS237_CHECK( glBindBuffer(GL_ARRAY_BUFFER, this->_insBuf) );
    CS237_CHECK( glBufferData(GL_ARRAY_BUFFER, sizeof(cs237::vec3f) * NUM_GRASS, &translations[0], GL_STATIC_DRAW) );


    CS237_CHECK( glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0) ); //3 * sizeof(GLfloat), (GLvoid*)0) );
    CS237_CHECK( glEnableVertexAttribArray(2) );
    CS237_CHECK( glVertexAttribDivisor(2, 1) );




    CS237_CHECK( glBindBuffer (GL_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindVertexArray (0) );

}

void GQuad::Draw() {

    // CS237_CHECK( glBindBuffer(GL_TEXTURE_BUFFER, _texBufObj) );
    // CS237_CHECK( glBindTexture(GL_TEXTURE_BUFFER, _texBuf) );



    CS237_CHECK(glActiveTexture(GL_TEXTURE3));
    texture->Bind();

    CS237_CHECK( glBindVertexArray (this->_id) );
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->_iBuf) );



    CS237_CHECK( glDrawElementsInstanced (GL_TRIANGLES, this->_nIndices, GL_UNSIGNED_INT, 0, NUM_GRASS) );




    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindVertexArray (0) );

}


