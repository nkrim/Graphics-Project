#include "skybox.hxx"


void SkyBox::Load() {
    // load all sides
    for (int i = 0; i < 6; i++) {
        sides[i].Load();
    }
}

void SkyBox::InitCoords(Camera cam) {
    cs237::vec3f* quadVerts[6] = {
        quadVertsLeft,
        quadVertsRight,
        quadVertsTop,
        quadVertsBottom,
        quadVertsNear,
        quadVertsFar
    };

    for (int i = 0; i < 6; i++) {
        sides[i].Copy(
                quadVerts[i],
                quadIndices,
                textCoords
            );

        for (int j = 0; j < 4; j++) {
            sides[i].verts[j] += d_to_f(cam.position());
        }
    }
}

void SkyBox::Draw() {
    // draw all sides
    for (int i = 0; i < 6; i++) {
        sides[i].Draw();
    }

}

void SkyBox::Init() {

    for (int i = 0; i < 6; i++) {
        sides[i].Init();
        cs237::image2d *img = new cs237::image2d("../data/sea-clouds/" + NAMES[i]);
        sides[i].texture = new cs237::texture2D(GL_TEXTURE_2D, img);

        sides[i].texture->Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        sides[i].texture->Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    delta = cs237::vec3f(0,0,0);


}

void Quad::Copy(cs237::vec3f* verts, uint32_t* indices, cs237::vec2f* texts) {

    for (int i = 0; i < 4; i++) {
        this->verts[i] = verts[i];
        this->indices[i] = indices[i];
        this->text[i] = texts[i];

    }

    for (int i = 4; i < 6; i++) {
        this->indices[i] = indices[i];
    }

}

void Quad::Init() {
    CS237_CHECK( glGenVertexArrays (1, &this->_id) );

    GLuint buf[3];
    CS237_CHECK( glGenBuffers (3, buf) );

    this->_vBuf = buf[0];
    this->_iBuf = buf[1];
    this->_tBuf = buf[2];
    this->_nIndices = 0;
}

void Quad::Load() {


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

    CS237_CHECK( glBindBuffer (GL_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindVertexArray (0) );

}

void Quad::Draw() {

    CS237_CHECK(glActiveTexture(GL_TEXTURE3));
    texture->Bind();

    CS237_CHECK( glBindVertexArray (this->_id) );
    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->_iBuf) );

    CS237_CHECK( glDrawElements (GL_TRIANGLES, this->_nIndices, GL_UNSIGNED_INT, 0) );

    CS237_CHECK( glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0) );
    CS237_CHECK( glBindVertexArray (0) );

}
