#include "viewer.h"

void free_array_with_alloced(void **list, const int size)
{
    if(list != NULL)
    {
        for(int i = 0; i < size; ++i)
        {
            if(list[i] != NULL)
            {
                free(list[i]);
                list[i] = NULL;
            }
        }

        free(list);
        list = NULL;

        printf("Free Success\n");
    }

}

int set_image_from_compressed_file(const char *file_name)
{
    // uncompress_data_set_t *tmp_list = (uncompress_data_set_t*)calloc(sizeof(uncompress_data_set_t), sizeof(uncompress_data_set_t));
    uncompressed_file_list = (uncompress_data_set_t*)calloc(sizeof(uncompress_data_set_t), sizeof(uncompress_data_set_t));
    int ret = load_from_zip(file_name, uncompressed_file_list);
    // printf("\ncompressed file load now\nsize: %d\n", tmp_list->size);

    if(!ret) {
        uncompressed_file_list = NULL;
        return FALSE;
    }

    for(int i = 0; i < uncompressed_file_list->size; i++) {
        printf("file name: %s, size: %d\n", uncompressed_file_list->uncompress_data_list[i]->file_name, uncompressed_file_list->uncompress_data_list[i]->file_size);
    }

    if(detail == NULL) {
        detail = (DirectoryDetail_t*)calloc(sizeof(DirectoryDetail_t), sizeof(DirectoryDetail_t));
    }

    detail->image_count = uncompressed_file_list->size;

    if(detail->image_count % 2)
    {
        detail->isOdd = TRUE;
    }
    else
    {
        detail->isOdd = FALSE;
    }

    return TRUE;
}

int get_image_file_count_from_directory(struct dirent **src, const int size, int *dst, const char *dirname)
{
    int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
    memset(number_list, 0, sizeof(int) * LIST_BUFFER);

    int dirname_length = strlen(dirname) + 1;

    int count = 1;
    for(int i = 0; i < size; ++i) {

        if(count < LIST_BUFFER) {

            int src_length = strlen(src[i]->d_name) + 1;
            char *final_path = (char*)calloc(dirname_length + src_length + 1, 1);
            if(final_path == NULL) {
                free(final_path);
                break;
            }

            strncat(final_path, dirname, dirname_length);
            char slash[1] = "/";
            strncat(final_path, slash, 1);
            strncat(final_path, src[i]->d_name, src_length);

            printf("%s\n", final_path);

            if(detect_image_from_file(final_path)) {
                number_list[count - 1] = i;
                count++;
                int *tmp = (int*)realloc(number_list, sizeof(int) * count);
                if(tmp != NULL) {
                    number_list = tmp;
                }
            }

            free(final_path);

        }

    }


    int *tmp = (int*)realloc(dst, sizeof(int) * count);
    if(tmp != NULL) {
        dst = tmp;
        memset(dst, 0, sizeof(int) * count);
        memcpy(dst, number_list, sizeof(int) * count);
    } else {
        free(tmp);
    }

    free(number_list);

    return count - 1;
}

int create_image_path_list(char **image_path_list, const char *dirname)
{
    struct dirent **file_list;

    printf("target directory: %s\n", dirname);

    int r = scandir(dirname, &file_list, NULL, alphasort);
    if(r < 1) {
        free(file_list);
        file_list = NULL;

        return 0;
    }

    int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
    memset(number_list, 0, sizeof(int) * LIST_BUFFER);

    int count = get_image_file_count_from_directory(file_list, r, number_list, dirname);

    if(count < 1) {

        free(number_list);
        number_list = NULL;

        free_array_with_alloced((void**)file_list, r);

        return 0;
    }

    printf("count: %d, r: %d\n", count, r);

    if(count < LIST_BUFFER) {
        size_t image_path_list_size = sizeof(char*) * count;
        char **tmp = (char**)realloc(image_path_list, image_path_list_size);

        if(tmp != NULL) {
            image_path_list = tmp;

            // memset(image_list, 0, image_list_size);

            for(int i = 0; i < count; ++i) {
                const int target = number_list[i];

                int dirname_length = strlen(dirname) + 1;
                const size_t target_length = strlen(file_list[target]->d_name) + 1;

                const size_t final_length = dirname_length + target_length + 1;
                char *final_path = (char*)calloc(final_length, 1);
                if(final_path == NULL) {
                    free(final_path);
                    break;
                }

                strncat(final_path, dirname, dirname_length);
                char slash[1] = "/";
                strncat(final_path, slash, 1);
                strncat(final_path, file_list[target]->d_name, target_length);

                printf("%s\n", file_list[target]->d_name);

                image_path_list[i] = (char*)calloc(dirname_length + target_length + 1, 1);

                strncpy(image_path_list[i], final_path, final_length);

                free(final_path);
            }
        } else {
            free(tmp);
        }
    }

    free(number_list);

    free_array_with_alloced((void**)file_list, r);

    return count;
}

int set_image_path_list(const char *dirname)
{
    if(detail != NULL) {
        detail = (DirectoryDetail_t*)malloc(sizeof(DirectoryDetail_t));

        if(detail != NULL) {
            memset(detail, 0, sizeof(DirectoryDetail_t));
        }
    }

    if(detail == NULL) {
        return 0;
    }

    if(detail->image_path_list == NULL) {
        detail->image_path_list = (char**)malloc(LIST_BUFFER);

        if(detail->image_path_list != NULL) {
            return 0;
        }
    }

    if(detail->image_path_list != NULL) {
        printf("Not NULL from detail->image_list\n");

        int count = create_image_path_list(detail->image_path_list, dirname);
        printf("image count: %d\n", count);
        if(count < 1) {
            return 0;
        }

        detail->image_count = count;

        printf("end\n");

        if(detail->image_count % 2) {
            detail->isOdd = TRUE;
        } else {
            detail->isOdd = FALSE;
        }

        return 1;
    }

    return 0;
}

void unref_dst()
{
    printf("unref_dst start\n");
    if(image_container_list[pages->current_page] != NULL) {
        if(pages->isSingle) {

            if(image_container_list[pages->current_page]->dst != NULL) {
                g_object_unref(G_OBJECT(image_container_list[pages->current_page]->dst));
            }

        } else {

            if(image_container_list[pages->current_page]->dst != NULL) {
                g_object_unref(G_OBJECT(image_container_list[pages->current_page]->dst));
            }

            if(image_container_list[pages->current_page - 1] != NULL && image_container_list[pages->current_page - 1]->dst != NULL) {
                g_object_unref(G_OBJECT(image_container_list[pages->current_page - 1]->dst));
            }
        }
    }
    printf("unref_dst end\n");
}

void next_image(int isForward)
{
    if(pages->isSingle) {
        unref_dst();

        set_image_container(pages->current_page);

        gtk_image_clear((GtkImage*)pages->left);

        gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);    

    } else {

        update_page(FALSE);

    }

}

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data)
{

    GdkModifierType consumed;
    const int ALL_ACCESS_MASK = GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK;

    GdkKeymap *keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    guint keyval;
    gdk_keymap_translate_keyboard_state(keymap, event->hardware_keycode, event->state, event->group, &keyval, NULL, NULL, &consumed);

    int isCtrl = (event->state & ~consumed & ALL_ACCESS_MASK) == GDK_CONTROL_MASK; 
    int isAlt = (event->state & ~consumed & ALL_ACCESS_MASK) == GDK_MOD1_MASK; 

    if(keyval == GDK_KEY_o && isCtrl) {
        open_file_on_menu();

        return TRUE;
    }

    if(keyval == GDK_KEY_q && isCtrl) {

        CloseWindow();

        return TRUE;
    }

    if(image_container_list == NULL) {
        return FALSE;
    }

    switch(event->keyval) {
        case GDK_KEY_Home:
            pages->current_page = 1;

            if(pages->isSingle)
            {
                pages->current_page = 0;
            }

            if(pages->page_direction_right)
            {
                next_image(FALSE);
            }
            else
            {
                next_image(TRUE);
            }

            return TRUE;

        default:
            break;
    }

    if(keyval == GDK_KEY_Return && isAlt) {
        fullscreen();

        return TRUE;
    }

    if(keyval == GDK_KEY_s && isCtrl) {
        change_spread_to_single();

        return TRUE;
    }

    if(keyval == GDK_KEY_d && isCtrl) {
        change_single_to_spread();

        return TRUE;
    }


    if((keyval == GDK_KEY_f && isCtrl) || event->keyval == GDK_KEY_Right || event->keyval == GDK_KEY_l) {

        move_left();

        printf("pressed right arrow key\n");

        return TRUE;
    }

    if((keyval == GDK_KEY_b && isCtrl) || event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_h) {

        move_right();

        printf("pressed right left key\n");

        return TRUE;

    }

    return FALSE;
}


void free_image_container()
{
    if(image_container_list != NULL) {
        for(int i = 0; i < detail->image_count; i++) {
            if(image_container_list[i] != NULL) {

                if(image_container_list[i]->aspect_raito != NULL) {
                    free(image_container_list[i]->aspect_raito);
                    image_container_list[i]->aspect_raito = NULL;
                }

                if(image_container_list[i]->src != NULL) {
                    g_object_unref(G_OBJECT(image_container_list[i]->src));
                }

                if(image_container_list[i]->dst != NULL) {
                    g_object_unref(image_container_list[i]->dst);
                }

                free(image_container_list[i]);
                image_container_list[i] = NULL;
            }
        }

        free(image_container_list);
        image_container_list = NULL;
    }

}

void close_variables()
{
    if(!window.isClose) {
        printf("free image_path_list\n");
        if(detail != NULL && detail->image_path_list != NULL) 
        {                                                                                
            free_array_with_alloced((void**)detail->image_path_list, detail->image_count);    
        }                                                                                

        printf("free image_container_list\n");
        free_image_container();

        printf("free uncompressed_file_list\n");
        free_uncompress_data_set(uncompressed_file_list);
        printf("end\n");

        if(detail != NULL) {
            free(detail);
            detail = NULL;
        }

        if(pages != NULL) {
            free(pages);
            pages = NULL;
        }

        window.isClose = TRUE;
        printf("Closed\n");
    }
}


void set_image_container(int position)
{
    if(image_container_list[position] == NULL) {
        image_container_list[position] = malloc(sizeof(Image_Container_t));
        memset(image_container_list[position], 0, sizeof(Image_Container_t));

        image_container_list[position]->err = NULL;

        if(!isCompressFile) {
            printf("gehahahah\n");

            if(detail->image_path_list[position] != NULL) {
                image_container_list[position]->src = gdk_pixbuf_new_from_file(detail->image_path_list[position], &image_container_list[position]->err);
            }

        } else {
            // printf("uncompressd load mode\n%d\n", uncompressed_file_list->uncompress_data_list[position]->file_size);

            GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
            gboolean check = gdk_pixbuf_loader_write(loader, uncompressed_file_list->uncompress_data_list[position]->data, uncompressed_file_list->uncompress_data_list[position]->file_size, NULL);
            if(!check) {
                printf("GdkPixbufLoader write error\n");
            }

            // memcpy(image_container_list[position]->src, gdk_pixbuf_loader_get_pixbuf(loader), uncompressed_file_list->uncompress_data_list[position]->file_size);
            image_container_list[position]->src = gdk_pixbuf_loader_get_pixbuf(loader);

            GError *err;
            if(loader != NULL) {
                printf("loader close begin\n");
                gdk_pixbuf_loader_close(loader, NULL);
                printf("loader close end\n");
            }

        }

        if(image_container_list[position]->src == NULL) {
            printf("empty!\n");
        }

        image_container_list[position]->src_width = gdk_pixbuf_get_width(image_container_list[position]->src);
        image_container_list[position]->src_height = gdk_pixbuf_get_height(image_container_list[position]->src);

        int width = image_container_list[position]->src_width;
        int height = image_container_list[position]->src_height;

        image_container_list[position]->aspect_raito = calc_aspect_raito(width, height, mygcd(width, height));

    }

    if(pages->isSingle)
    {
        resize_when_single(position);
    }
}

void resize_when_single(int position)
{
    gint window_width = 0;
    gint window_height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
    window.width = window_width;
    window.height = window_height;

    int width = image_container_list[position]->src_width;
    int height = image_container_list[position]->src_height;

    double w_aspect = (double)image_container_list[position]->aspect_raito[0];
    double h_aspect = (double)image_container_list[position]->aspect_raito[1];

    /*
       printf("widow size w: %d h: %d\n", window_width, window_height);
       printf("src w: %d h: %d\n", width, height);
       printf("%f : %f\n", w_aspect, h_aspect);
       */

    if(height > window_height) {
        int diff = height - window_height;
        height = height - diff;
        int result = (int)ceil((double)height * (w_aspect / h_aspect));
        // printf("result: %d, %d * %f\n", result, height, (w_aspect / h_aspect));
        width = result;
    }

    /*
       if(width > window_width)
       {
       int diff = width - window_width;
       width = width - diff;
       height = (int)ceil((double)width * (h_aspect / w_aspect));
       }
       */

    /*
       if(image_container_list[position]->dst != NULL) {
       g_object_unref((GtkWidget*)image_container_list[position]->dst);
       }
       */


    image_container_list[position]->dst = gdk_pixbuf_scale_simple(image_container_list[position]->src, width, height, GDK_INTERP_BILINEAR);
    image_container_list[position]->dst_width = width;
    image_container_list[position]->dst_height = height;
}

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    if(!isFirstLoad) {
        gint width = 0;
        gint height = 0;
        gtk_window_get_size((GtkWindow*)window.window, &width, &height);

        if(width != window.width || height != window.height) {
            if(pages->isSingle) {

                unref_dst();

                resize_when_single(pages->current_page);

                gtk_image_clear((GtkImage*)pages->left);

                gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);


            } else {
                if(detail->isOdd && detail->image_count == pages->current_page - 1) {

                    unref_dst();

                    resize_when_single(pages->current_page);

                    gtk_image_clear((GtkImage*)pages->left);
                    gtk_image_clear((GtkImage*)pages->right);

                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                    // g_object_unref(G_OBJECT(image_container_list[pages->current_page]->dst));

                } else {
                    /*
                       resize_when_single(pages->current_page - 1);
                       resize_when_single(pages->current_page);
                       */

                    unref_dst();

                    int isOverHeight = resize_when_spread(pages->current_page);

                    gtk_image_clear((GtkImage*)pages->left);
                    gtk_image_clear((GtkImage*)pages->right);

                    if(pages->page_direction_right) {
                        gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                        gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page - 1]->dst);
                    } else {
                        gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page - 1]->dst);
                        gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
                    }

                    set_margin_left_page(pages->current_page, isOverHeight, FALSE);


                    // g_object_unref(G_OBJECT(image_container_list[pages->current_page]->dst));
                    // g_object_unref(G_OBJECT(image_container_list[pages->current_page - 1]->dst));


                }
            }

            return TRUE;
        }
    }


    return FALSE;
}

void set_image(GtkWidget **img, int position)
{
    *img = gtk_image_new_from_pixbuf(image_container_list[position]->dst);
}

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth)
{
    if(isOverWidth)
    {
        int diff = *x - window_width;
        int width = *x - diff;
        int height = (int)ceil((double)width * (h_aspect / w_aspect));

        *x = width;
        *y = height;
    }
    else
    {
        int diff = *y - window_height;
        int height = *y - diff;
        int width = (int)ceil((double)height * (w_aspect / h_aspect));

        *x = width;
        *y = height;
    }


}

int resize_when_spread(int page)
{
    int left_src_width = image_container_list[page - 1]->src_width;
    int left_src_height = image_container_list[page - 1]->src_height;
    double left_x_aspect = (double)image_container_list[page - 1]->aspect_raito[0];
    double left_y_aspect = (double)image_container_list[page - 1]->aspect_raito[1];

    int right_src_width = image_container_list[page]->src_width;
    int right_src_height = image_container_list[page]->src_height;
    double right_x_aspect = (double)image_container_list[page]->aspect_raito[0];
    double right_y_aspect = (double)image_container_list[page]->aspect_raito[1];

    gint window_width = 0;
    gint window_height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
    window.width = window_width;
    window.height = window_height;

    int half_width = window_width / 2;

    /*
       if(draw_area.width > 0)
       {
       half_width = draw_area.width / 2;
       window_width = draw_area.width;
       window_height = draw_area.height;
       }
       */

    int left_width = half_width;
    int left_height = 0;
    left_height = (int)ceil((double)left_width * (left_y_aspect / left_x_aspect));
    int isOverHeight = FALSE;
    if(left_height > window_height)
    {
        scale_when_oversize(&left_width, &left_height, window_width, window_height, left_x_aspect, left_y_aspect, FALSE);
        isOverHeight = TRUE;
    }

    image_container_list[page - 1]->dst_width = left_width;
    image_container_list[page - 1]->dst_height = left_height;

    printf("Left Width: %d, Left Height: %d\n", left_width, left_height);

    int right_width = half_width;
    int right_height = 0;
    right_height = (int)ceil((double)right_width * (right_y_aspect / right_x_aspect));

    if(right_height > window_height)
    {
        scale_when_oversize(&right_width, &right_height, window_width, window_height, right_x_aspect, right_y_aspect, FALSE);
    }

    image_container_list[page]->dst_width = right_width;
    image_container_list[page]->dst_height = right_height;

    image_container_list[page - 1]->dst = gdk_pixbuf_scale_simple(image_container_list[page - 1]->src, left_width, left_height, GDK_INTERP_BILINEAR);
    image_container_list[page]->dst = gdk_pixbuf_scale_simple(image_container_list[page]->src, right_width, right_height, GDK_INTERP_BILINEAR);

    return isOverHeight;
}

void set_margin_left_page(int position, int isOverHeight, int isFinalPage)
{
    /*
       gint window_width = 0;
       gint window_height = 0;
       gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
       window.width = window_width;
       window.height = window_height;
       */

    if(isOverHeight) {
        int mix_width = (image_container_list[position - 1]->dst_width + image_container_list[position]->dst_width);
        int margin_left = (fmax(mix_width, window.width) - fmin(mix_width, window.width)) / 2;

        printf("margin left: %d\n", margin_left);

        // g_object_set(pages->left, "margin_left", margin_left, NULL);

        if(isFinalPage) {
            gtk_widget_set_margin_start(pages->right, image_container_list[position]->dst_width);
        } else {
            gtk_widget_set_margin_start(pages->left, (gint)margin_left);
        }
    } else {
        // g_object_set(pages->left, "margin_left", 0, NULL);

        if(isFinalPage) {
            gtk_widget_set_margin_start(pages->right, 0);
        } else {
            gtk_widget_set_margin_start(pages->left, 0);
        }
    }

}

int init_image_object(const char *file_name, int startpage)
{
    pages->current_page = 0;
    // set image file path
    if(!isFirstLoad) {
        free_array_with_alloced((void**)detail->image_path_list, detail->image_count);    
        detail->image_path_list = NULL;

        free_uncompress_data_set(uncompressed_file_list);
        uncompressed_file_list = NULL;

        free_image_container();
        image_container_list = NULL;

        detail->image_count = 0;

        gtk_image_clear((GtkImage*)pages->left);
        gtk_image_clear((GtkImage*)pages->right);

        // pages->current_page = 0;
    }


    if(isCompressFile) {

        if(!set_image_from_compressed_file(file_name)) {
            return FALSE;
        }

    } else {
        if(!set_image_path_list(file_name)) {
            return FALSE;
        }
    }



    if(detail->image_count > 0)
    {
        image_container_list = (Image_Container_t**)calloc(detail->image_count, sizeof(Image_Container_t*));

        if(pages->isSingle || detail->image_count == 1)
        {
            set_image_container(0);

            resize_when_single(0);

            set_image(&pages->left, 0);
        }
        else
        {
            // printf("%s\n%s\n", detail->image_path_list[0], detail->image_path_list[1]);
            set_image_container(0);
            set_image_container(1);

            /*
               int result_width = image_container_list[0]->src_width + image_container_list[1]->src_width + 1;
               int result_height = image_container_list[0]->src_height + image_container_list[1]->src_height + 1;

               gdk_pixbuf_copy_area(image_container_list[0]->src, image_container_list[0]->src_width, image_container_list[0]->src_width + 1, image_container_list[0]->src_height + 1, image_container_list[0]->src_height, image_container_list[0]->src, result_width, result_height);

*/
            resize_when_spread(1);

            if(pages->page_direction_right)
            {
                set_image(&pages->left, 1);
                set_image(&pages->right, 0);
            }
            else
            {
                set_image(&pages->left, 0);
                set_image(&pages->right, 1);
            }


            pages->current_page = 1;
        }

        return TRUE;
    }

    return FALSE;
}


void fullscreen()
{
    if(window.isFullScreen)
    {
        gtk_window_unfullscreen(GTK_WINDOW(window.window));
        gtk_widget_show(window.menubar);
        window.isFullScreen = FALSE;
    }
    else
    {
        gtk_window_fullscreen(GTK_WINDOW(window.window));
        gtk_widget_hide(window.menubar);
        window.isFullScreen = TRUE;
    }

}

void update_page(int isSingleChange)
{
    if(!isFirstLoad) {

        if(isSingleChange) {

            if(pages->isSingle) {

                if(detail->isOdd && pages->current_page == (detail->image_count - 1)) {

                    gtk_image_clear(GTK_IMAGE(pages->right));

                } else {

                    if(pages->left != NULL) {
                        gtk_image_clear(GTK_IMAGE(pages->left));
                    }

                    if(pages->right != NULL) {
                        gtk_image_clear(GTK_IMAGE(pages->right));
                    }

                }

                pages->current_page -= 1;

                if(pages->current_page < 0) {
                    pages->current_page = 0;
                }

                resize_when_single(pages->current_page);

                set_image(&pages->left, pages->current_page);

                update_grid();

            } else {

                gtk_image_clear(GTK_IMAGE(pages->left));

                pages->current_page++;

                if(pages->current_page >= detail->image_count - 1) {
                    pages->current_page--;
                }

                resize_when_spread(pages->current_page);

                set_image(&pages->left, pages->current_page);
                set_image(&pages->right, pages->current_page - 1);

                update_grid();
            }
        } else {

            set_image_container(pages->current_page);
            set_image_container(pages->current_page - 1);

            if(pages->right != NULL) {
                gtk_image_clear(GTK_IMAGE(pages->right));
            }

            if(pages->left != NULL) {
                gtk_image_clear(GTK_IMAGE(pages->left));
            }


            if(detail->isOdd && (pages->current_page == detail->image_count - 1)) {

                unref_dst();

                // resize_when_single(pages->current_page);

                int isOverHeight = resize_when_spread(pages->current_page);

                if(pages->page_direction_right) {

                    gtk_image_set_from_pixbuf(GTK_IMAGE(pages->right), image_container_list[pages->current_page]->dst);

                } else {

                    gtk_image_set_from_pixbuf(GTK_IMAGE(pages->left), image_container_list[pages->current_page]->dst);
                }

                // set_margin_left_page(pages->current_page, isOverHeight, TRUE);

            } else {


                unref_dst();

                int isOverHeight = resize_when_spread(pages->current_page);

                if(pages->page_direction_right)
                {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(pages->right), image_container_list[pages->current_page - 1]->dst);
                    gtk_image_set_from_pixbuf(GTK_IMAGE(pages->left), image_container_list[pages->current_page]->dst);
                }
                else
                {
                    gtk_image_set_from_pixbuf(GTK_IMAGE(pages->left), image_container_list[pages->current_page - 1]->dst);
                    gtk_image_set_from_pixbuf(GTK_IMAGE(pages->right), image_container_list[pages->current_page]->dst);
                }

                set_margin_left_page(pages->current_page, isOverHeight, FALSE);
            }

        }
    }
}

void update_grid()
{
    if(pages->isSingle) {
        if(!isFirstLoad) {
            gtk_grid_remove_column(GTK_GRID(grid), 0);
        }

        gtk_widget_set_vexpand(pages->left, TRUE);
        gtk_widget_set_hexpand(pages->left, TRUE);
        gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, 1, 1);

        gtk_widget_show(pages->left);

    } else {

        printf("update_grid spread\n");

        if(!isFirstLoad) {
            gtk_grid_remove_column(GTK_GRID(grid), 0);
            gtk_grid_remove_column(GTK_GRID(grid), 0);
        }

        // gtk_widget_set_hexpand(pages->left, TRUE);
        gtk_widget_set_vexpand(pages->left, TRUE);

        // gtk_widget_set_hexpand(pages->right, TRUE);
        gtk_widget_set_vexpand(pages->right, TRUE);

        gtk_grid_attach(GTK_GRID(grid), pages->left, 0, 0, 1, 1);

        gtk_grid_attach_next_to(GTK_GRID(grid), pages->right, pages->left, GTK_POS_RIGHT, 1, 1);

        // gtk_grid_attach(GTK_GRID(grid), pages->right, 1, 0, 1, 1);

        gtk_widget_show(pages->left);
        gtk_widget_show(pages->right);


    }

    if(isFirstLoad) {
        isFirstLoad = FALSE;
    }
}

GtkWidget *create_menu_bar()
{

    // settings menubar
    GtkWidget *menubar = gtk_menu_bar_new();

    // File Menu
    file_menu_struct.body = gtk_menu_new();
    file_menu_struct.root = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_struct.root), file_menu_struct.body);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_menu_struct.root);

    file_menu_struct.load = gtk_menu_item_new_with_label("Load");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu_struct.body), file_menu_struct.load);
    g_signal_connect(G_OBJECT(file_menu_struct.load), "activate", G_CALLBACK(open_file_on_menu), NULL);

    file_menu_struct.quit = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu_struct.body), file_menu_struct.quit);
    g_signal_connect(G_OBJECT(file_menu_struct.quit), "activate", G_CALLBACK(CloseWindow), NULL);

    // View Menu
    view_menu_struct.body = gtk_menu_new();
    view_menu_struct.root = gtk_menu_item_new_with_label("View");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_struct.root), view_menu_struct.body);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view_menu_struct.root);

    view_menu_struct.page_direction = gtk_menu_item_new_with_label("Change Page Dpirection");
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.page_direction);
    g_signal_connect(G_OBJECT(view_menu_struct.page_direction), "activate", G_CALLBACK(change_direction), NULL);


    view_menu_struct.set_single_mode = gtk_menu_item_new_with_label("Set Single Mode");
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.set_single_mode);
    g_signal_connect(G_OBJECT(view_menu_struct.set_single_mode), "activate", G_CALLBACK(change_spread_to_single), NULL);

    view_menu_struct.set_spread_mode = gtk_menu_item_new_with_label("Set Spread Mode");
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu_struct.body), view_menu_struct.set_spread_mode);
    g_signal_connect(G_OBJECT(view_menu_struct.set_spread_mode), "activate", G_CALLBACK(change_single_to_spread), NULL);

    // Help Menu
    help_menu_struct.body = gtk_menu_new();
    help_menu_struct.root = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_struct.root), help_menu_struct.body);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_menu_struct.root);

    help_menu_struct.about = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu_struct.body), help_menu_struct.about);

    return menubar;
}


void move_left()
{
    if(pages->isSingle) {
        pages->current_page++;
    } else if(pages->page_direction_right) {

        if(detail->isOdd && pages->current_page == detail->image_count - 1)  {
            pages->current_page--;
        } else {
            pages->current_page -= 2;
        }

    } else {

        if(detail->isOdd && pages->current_page == detail->image_count - 1) {
            pages->current_page = 0;
        } else {
            pages->current_page += 2;
        }

    }

    if(pages->current_page >= detail->image_count) {
        if(pages->isSingle) {
            pages->current_page = 0;
        } else {
            pages->current_page = 1;
        }
    }

    if(pages->current_page < 0) {
        pages->current_page = detail->image_count - 1;
    }

    // printf("%s\n", detail->image_path_list[pages->current_page]);

    if(pages->page_direction_right) {
        next_image(FALSE);
    } else {
        next_image(TRUE);
    }

}


void move_right()
{
    if(pages->isSingle) {
        pages->current_page--;
    } else if(pages->page_direction_right) {

        pages->current_page += 2;

        if(pages->current_page == detail->image_count) {
            pages->current_page--;
        }

    } else {

        if(detail->isOdd && pages->current_page == detail->image_count - 1) {
            pages->current_page--;
        } else {
            pages->current_page -= 2;
        }
    }

    if(pages->current_page >= detail->image_count) {
        if(pages->isSingle) {
            pages->current_page = 0;
        } else {
            pages->current_page = 1;
        }
    }

    if(pages->current_page < 0) {
        pages->current_page = detail->image_count - 1;
    }

    printf("current_page: %d\n", pages->current_page);

    if(pages->page_direction_right) {
        next_image(TRUE);
    } else {
        next_image(FALSE);
    }

}
