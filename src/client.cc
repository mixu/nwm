Client* Client::getByWindow(Monitor* monit, Window win) {
  Client *c;
  for(c = monit->clients; c; c = c->next)
    if(c->win == win)
      return c;
  return NULL;
}

Client* Client::getById(Monitor* monit, int id) {
  Client *c;
  for(c = monit->clients; c; c = c->next)
    if(c->id == id)
      return c;
  return NULL;  
}

void Client::attach() {
  this->next = this->mon->clients;
  this->mon->clients = this;  
}

void Client::detach(){
  Client **tc;
  for(tc = &this->mon->clients; *tc && *tc != this; tc = &(*tc)->next);
  *tc = this->next;      
}

