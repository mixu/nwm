
Client* Client::getByWindow(std::vector <Monitor>* monits, Window win) {
  unsigned int i, j;
  for(i = 0; i < monits->size(); i++) {
    for(j = 0; j < monits->at(i).clients.size(); j++) {
      if(monits->at(i).clients[j].win == win)
        return &monits->at(i).clients[j];
    }
  }
  return NULL;
}

Client* Client::getById(std::vector <Monitor>* monits, int id) {
  unsigned int i, j;
  for(i = 0; i < monits->size(); i++) {
    for(j = 0; j < monits->at(i).clients.size(); j++) {
      if(monits->at(i).clients[j].id == id)
        return &monits->at(i).clients[j];
    }
  }
  return NULL;  
}
