#include <iostream>
#include <vector>

using v_flp = std::vector<float*>;

class splice: public v_flp
{
  public:
    bool operator== (splice& Other);
};

bool splice::operator== (splice& Other)
{
  size_t n = this->size();
  if(Other.size() != n) return false;

  for(size_t i=0; i < n; ++i) if( *((*this)[i]) != *Other[i] ) return false;
  return true;
}


int main(int, char**)
{
  float v1[3] {0.1f, 0.2f, 0.3f};
  float v2[3] {0.1f, 0.2f, 0.3f};

  splice S1 {};
  S1.clear();
  S1.resize(3);

  S1[0] = &v1[0];
  S1[1] = &v1[1];
  S1[2] = &v1[2];

  splice S2 {};
  S2.clear();
  S2.resize(3);

  S2[0] = &v2[0];
  S2[1] = &v2[1];
  S2[2] = &v2[2];

  std::cout << (S1 == S2) << "\n";
  return true;
}
