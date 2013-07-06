#ifndef NODE_H
#define NODE_H

class Node
{
  private:
    long id;
    int type;
    int voltage;
    
  public:
    void init(long id, int type, S_message message);
    
    int get_voltage();
    
    
}

#endif
