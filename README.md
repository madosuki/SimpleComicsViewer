# SimpleComicsViewer

## This Program is so buggy.

### Depend
- **GTK3**
- **GDK3**
- **zlib**

### LICENCE
**GPLv3**

### Feature
- **Load Image**
- **Load Compressed File**
- **Show Spread**

### Supported file format
- **Single File**
    - JPEG
    - PNG
- **Compressed File**
    - PNG or JPEG inside of non encrypted zip

### Manual
- **Shortcut Key**
    - **Move to right**  
        l or right arrow or Ctrl+f
    - **Move to left**  
        h or left arrow or Ctrl+b
    - **Open File**  
        Ctrl+o
    - **Quit**  
        Ctrl+q or Alt+F4
    - **Change Single to Sprad**  
        Ctrl+d
    - **Change Spread to Single**  
        Ctrl+s

## How to run

- **Use AppImage**  
First download AppImage from release page.  
```
chmod +x ./Simple_Comics_Viewer-x86_64.AppImage
./Simple_Comics_Viewer-x86_64.AppImage
```

- **After build**
```
./build/simple_comics_viewer
```

### How to Build
```
sudo apt-get install libgtk-3-dev build-essential
make
```
