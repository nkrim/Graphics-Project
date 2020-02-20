#include "cs237.hxx"
#include "view.hxx"
#include "camera.hxx"
#include "map-cell.hxx"
#include "buffer-cache.hxx"
#include "texture-cache.hxx"

float View::FindHeight() {
    return FindHeightAt(_cam.position()) + 40.0f;
}

float View::FindHeightAt(cs237::vec3d pos) {
    for (auto r = 0; r < _map->nRows(); r++) {
        for (auto c = 0; c < _map->nCols(); c++) {
            // this is a cell
            class Cell *cell = _map->Cell(r, c);


            cs237::vec3d campos = (pos) - cs237::vec3d(cell->Col() * cell->Width() * cell->hScale(), 0.0f, cell->Row() * cell->Width() * cell->hScale());
            // printf("%f, %f| %f, %f\n", cell->Row() * cell->Width() * cell->hScale(), cell->Col() * cell->Width() * cell->hScale(), campos[0], campos[2]);
            // recurse through cell's root tile to find appropriate LOD to render
            // get the vector of tiles to make vaos for
            std::vector<Tile *> tiles = this->GetAllTiles(&cell->Tile(0));
            // assign vaos with each tile's data

            // TODO: what tiles do I go over?
            for(auto it = tiles.begin(); it != tiles.end(); it++) {
                Tile* tile = *it;

                if (PointInAABBxz(tile->BBox(), pos)) {

                    bool flip = false;

                    float height = 0;

                    for (auto i = 0; i < tile->Chunk()._nIndices - 2; i++) {

                        int i1 = (int) tile->Chunk()._indices[i];
                        int i2 = (int) tile->Chunk()._indices[i+1];
                        int i3 = (int) tile->Chunk()._indices[i+2];

                        if (i1 == i2 || i2 == i3 || i1 == i3) {
                            // degenerate triangle
                            continue;
                        }
                        if (i3 == 0xffff || i2 == 0xffff || i1 == 0xffff) {
                            // prim restart
                            // further triangles are in skirt
                            break;
                        }

                        cs237::vec3f v1 = vec3fFromVertex(tile->Chunk()._vertices[i1]);
                        cs237::vec3f v2, v3;
                        // printf("%f, %f, %f\n", (float)tile->Chunk()._vertices[i3]._y, tile->Chunk()._vertices[i3]._y * _map->vScale(), campos);
                        if (flip) {
                            v2 = vec3fFromVertex(tile->Chunk()._vertices[i3]);
                            v3 = vec3fFromVertex(tile->Chunk()._vertices[i2]);
                        } else {
                            v2 = vec3fFromVertex(tile->Chunk()._vertices[i2]);
                            v3 = vec3fFromVertex(tile->Chunk()._vertices[i3]);
                        }

                        v1 *= this->_scale;
                        v2 *= this->_scale;
                        v3 *= this->_scale;

                        cs237::vec2f point = vec3fxz(d_to_f(campos));
                        cs237::vec2f vert1 = vec3fxz(v1);
                        cs237::vec2f vert2 = vec3fxz(v2);
                        cs237::vec2f vert3 = vec3fxz(v3);

                        if (PointInTriangle( point, vert1, vert2, vert3 )) {

                            cs237::vec3f N = normalize(cross(v3 - v2, v1 - v2));
                            height = dot( (v2 - cs237::vec3f(point[0], 0.0f, point[1])), N ) / dot(cs237::vec3f(0.0f, 1.0f, 0.0f), N);

                            // printf("%f, %f | heignt:%f\n\n",campos[0], campos[2], height);
                            return height;
                        }

                        flip = !flip;
                    }

                }
            }
        }
    }
    return -100.0f;
}