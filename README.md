# SimpleComicsViewer

## This Program is so buggy.

### Depends List
- **GTK3**
- **GDK3**
- **zlib**
- **[MuPDF](https://github.com/ArtifexSoftware/mupdf)**

### LICENCE
**GPLv3**

### Features
- **Load Image(Currently support only JPEG and PNG)**
- **Load Compressed File(Currently support only zip)**
- **Show Spread**

### Supported file format
- **Single File**
    - JPEG
    - PNG
    - PDF(but still experimental support)  
- **Compressed File**
    - PNG or JPEG inside of non encrypted zip

### Manual
- **Shortcut Key**
    - **Move to Right**  
        l or right arrow or Ctrl+f  
    - **Move to Left**  
        h or left arrow or Ctrl+b  
    - **Move to Top Page**  
        Home key or Ctrl+a or 0  
    - **Move to End Page**  
        End key or Ctrl+e or Shift+$  
    - **Open File**  
        Ctrl+o  
    - **Quit**  
        Ctrl+q or Alt+F4  
    - **Change Single to Sprad**  
        Ctrl+d  
    - **Change Spread to Single**  
        Ctrl+s  
    - **Change priority to Cover**  
        c  

## How to run

- **Use AppImage**  
First download AppImage from release page.  
```
chmod +x ./Simple_Comics_Viewer-x86_64.AppImage
./Simple_Comics_Viewer-x86_64.AppImage
```

- **After build**
```
./src/simple_comics_viewer
```

### How to Build
```
sudo apt-get install libgtk-3-dev build-essential automake autoconf
```
After move to project dir.
```
./autogen.sh && ./configure && make
```
