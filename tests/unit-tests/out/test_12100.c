_Owner int socket();
void close(_Owner int fd);

int main()
{
  _Owner int fd;
  
  fd = socket();
  if (fd < 0)
  {
        
     return 1;
  }
  close(fd);
}

