
class Bounds {
public:
  Bounds() {
    bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
  }

  Bounds(double minx, double miny, double maxx, double maxy) {
    setBounds(minx, miny, maxx, maxy);
  }

  inline void setBounds(double minx, double miny, double maxx, double maxy) {
    bounds[0] = minx;
    bounds[1] = miny;
    bounds[2] = maxx;
    bounds[3] = maxy;
  }

  inline double getMinX() {
    return bounds[0];
  }
  inline double getMinY() {
    return bounds[1];
  }
  inline double getMaxX() {
    return bounds[2];
  }
  inline double getMaxY() {
    return bounds[3];
  }

  inline double getWidth() {
    return getMaxX() - getMinX();
  }

  inline double getHeight() {
    return getMaxY() - getMinY();
  }

  inline Bounds * getSW() {
    return new Bounds(getMinX(),
                      getMinY(),
                      getMinX() + (getWidth() / 2),
                      getMinY() + (getHeight() / 2));
  }
  inline Bounds * getNW() {
    return new Bounds(getMinX(),
                      getMaxY() - (getHeight() / 2),
                      getMinX() + (getWidth() / 2),
                      getMaxY());
  }
  inline Bounds * getNE() {
    return new Bounds(getMaxX() - (getWidth() / 2),
                      getMaxY() - (getHeight() / 2),
                      getMaxX(),
                      getMaxY());
  }
  inline Bounds * getSE() {
    return new Bounds(getMaxX() - (getWidth() / 2),
                      getMinY(),
                      getMaxX(),
                      getMinY() + (getHeight() / 2));
  }
  
  inline bool overlaps(Bounds *other) {
    // see
    // <http://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other>
    return getMinX() < other->getMaxX() && other->getMinX() < getMaxX() &&
           getMinY() < other->getMaxY() && other->getMinX() < getMaxY();
  }
  
private:
  double bounds[4];
};
