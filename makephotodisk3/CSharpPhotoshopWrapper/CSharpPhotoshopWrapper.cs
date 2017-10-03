/**************************************************************************************
Copyright(C) 2016, Charles C. Sentz. All Rights Reserved.


    This is the implementation of the C# wrapper for Photoshop. A C++ app needs to 
    instantiate a CLRPhotoshopWrapper object and call us through the paralel member functions
    in that class. 

    A C++ app compiled with CLR can call the class implemented in this DLL using the same 
    techniques illustrated in CLRPhotoshopWrapper
 
    All functions and data-members are class statics. There should never be more than one 
    instance of this class in existence. 
 
***************************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics ;
using Photoshop;


// need to define matching error-codes for C++ 
public enum ErrorCode
{
    NoError = 0,
    PhotoshopNotFound,                              // unable to load Photoshop application
    NoDiskRootSet,                                  // need to specify the disk root directory before calling OutputJpeg 
    NoActiveSourceDoc,                              // need to call LoadAndPrepareSourceFile before calling OutputJpeg
    AlreadyHaveActiveSourceDoc,                     // calling LoadAndPrepareSourceFile with an active source file (need to call UnloadSourceFile)
    IllegalCall,                                    // generic error 
} ;

// need to ensure these match #defines for C++ 
public enum JpegType
{
    Thumbnail = 0,
    Presentation,
    MiniPresentation,
    FullSize,
    Facebook,
    Compressed,
} ;




public class CSharpPhotoshopWrapper
{
    static Photoshop.Application s_app = null;
    static ErrorCode s_error_code = ErrorCode.NoError;
    static bool s_source_loaded = false;
    static string s_disk_root = null;
    static string s_dest_filename = null;


    /*
     * ---------------------------------------------------------------------------------------
     * LoadApp
     * 
     * PRIVATE 
     * 
     * member which Loads the photoshop application if required
     * 
     */
    private static bool LoadApp()
    {
        bool isOK = true;

        /*
            * I've seen Photoshop just crap out on a random basis when starting... but
            * then it works fine on a second try. So this looks like a kludge, and frankly,
            * it is a kludge... but it's about the best we can do for now and it does work
            */
        try
        {
            s_app = new Photoshop.Application();
        }
        catch
        {
            try
            {
                s_app = new Photoshop.Application();
            }
            catch
            {
                isOK = false;
                s_error_code = ErrorCode.PhotoshopNotFound;
            }
        }

        return isOK;
    }



    /*
     * ---------------------------------------------------------------------------------------
     * ProcessJpeg
     * 
     * PRIVATE
     * 
     * member to output an individual jpeg of the photoshop document passed in with designated script, quality, destination
     * 
    */
    private static bool ProcessJpeg(Photoshop.Document ps_document, string action_name, int jpeg_quality, string subdir, ref int width, ref int height )
    {
        bool isOK = false;
        Photoshop.Document work_document;

        Debug.Assert(s_app != null, "OutputJpeg called in spite of s_app being null.");
        Debug.Assert(ps_document != null, "OutputJpeg called with a null work document.");
        Debug.Assert(s_app.ActiveDocument == ps_document, "OutputJpeg called with inactive document");

        work_document = ps_document.Duplicate();

        Debug.Assert(s_app.ActiveDocument == work_document, "Duplicate document didn't become active document?");

        if (s_app != null && work_document != null)
        {
            PsUnits save_ruler ;

            s_app.DoAction(action_name, "Presentation");

            Photoshop.JPEGSaveOptions save_options = new Photoshop.JPEGSaveOptions();

            save_options.Quality = jpeg_quality;
            save_options.EmbedColorProfile = true;
            save_options.Matte = PsMatteType.psNoMatte;
            save_options.FormatOptions = PsFormatOptionsType.psStandardBaseline;

            work_document.SaveAs(s_disk_root + "\\" + subdir + "\\" + s_dest_filename, save_options);

            save_ruler = s_app.Preferences.RulerUnits ;
            s_app.Preferences.RulerUnits = PsUnits.psPixels ;

            // need to return the final dimensions in pixels. Width and Height are in inches! 
            height = (int) work_document.Height ;
            width = (int) work_document.Width ;

            s_app.Preferences.RulerUnits = save_ruler ;

            work_document.Close(Photoshop.PsSaveOptions.psDoNotSaveChanges);

            // all the photoshop methods are void... so we just have to assume it all worked OK 
            isOK = true;
        }

        return isOK;
    }



    /*
     * =======================================================================================
     * SetDiskRoot
     * 
     * PUBLIC
     * 
     *      Call this first, with the absolute path to the root directory for the disk project
     *      on the hard drive. 
     *      
     *      Right now, always succeeds. In the future, may also deal converting relative paths
     *      to absolute paths, since Photoshop doesn't seem to handle relative paths 
     */
    public bool SetDiskRoot(string disk_root)
    {
        bool isOK = true;

        s_disk_root = disk_root;

        return isOK;
    }



    /*
     * =======================================================================================
     * LoadAndPrepareSourceFile
     * 
     * PUBLIC 
     * 
     * API to call to begin processing a new image source file. Next, the user calls 
     * OutputJpeg for each desired jpeg type - thumbnail, presentation, fullsize, etc. 
     * 
     */
    public static bool LoadAndPrepareSourceFile(string source_file, string dest_filename)
    {
        bool isOK = false;

        if (s_app != null || LoadApp())
        {
            Debug.Assert(s_app != null, "LoadApp reported success but s_app still null?!?!");

            if (!s_source_loaded)
            {
                s_source_loaded = true;

                Photoshop.Document base_document;
                int channel_no;

                base_document = s_app.Open(source_file);
                s_app.ActiveDocument = base_document;

                // zap any non-RGB channels. Don't need to worry about paths 
                channel_no = 1;

                while (channel_no <= base_document.Channels.Count)
                {
                    if (base_document.Channels[channel_no].Kind != PsChannelType.psComponentChannel)
                        base_document.Channels[channel_no].Delete();
                    else
                        channel_no++;
                }

                // create and run a script to execute the deleteAllAnnot action, which deletes any "notes" 
                string script = "executeAction( stringIDToTypeID( \"deleteAllAnnot\" ) ) ;";
                s_app.DoJavaScript(script, null, null);

                // flatten image and convert to 8-bit RGB in the sRGB color space 
                s_app.DoAction("PrepareImage", "Presentation");

                s_dest_filename = dest_filename;

                isOK = true;
            }
            else
            {
                s_error_code = ErrorCode.AlreadyHaveActiveSourceDoc;
            }
        }

        return isOK;
    }



    /*
     * =======================================================================================
     * OutputJpeg
     * 
     * PUBLIC
     * 
     * API to call in order to output an individual jpeg type using the currently active source
     * document. 
     * 
     * This call is only legal after the user has called LoadAndPrepareSourceFile
     * 
     */
    public static bool OutputJpeg(JpegType jpeg_type, ref int width, ref int height )
    {
        bool isOK = false;

        if (s_app != null)
        {
            if (s_source_loaded)
            {
                if (s_disk_root != null)
                {
                    Photoshop.Document base_document;

                    base_document = s_app.ActiveDocument;

                    if (base_document != null)
                    {
                        bool vertical;

                        vertical = (base_document.Height > base_document.Width);

                        switch (jpeg_type)
                        {
                            case JpegType.Presentation:
                                isOK = ProcessJpeg(base_document, vertical ? "Vertical PRESENTATION" : "Horizontal PRESENTATION", 12, "images\\presentation", ref width, ref height);
                                break;

                            case JpegType.MiniPresentation:
                                isOK = ProcessJpeg(base_document, vertical ? "Small Vertical PRESENTATION" : "Small Horizontal PRESENTATION", 12, "images\\minip", ref width, ref height);
                                break;

                            case JpegType.Facebook:
                                isOK = ProcessJpeg(base_document, vertical ? "Vertical FACEBOOK" : "Horizontal FACEBOOK", 5, "images\\facebook", ref width, ref height );
                                break;

                            case JpegType.FullSize:
                                isOK = ProcessJpeg(base_document, vertical ? "Vertical FULL" : "Horizontal FULL", 12, "images\\fullsize", ref width, ref height);
                                break;

                            case JpegType.Compressed:
                                isOK = ProcessJpeg(base_document, vertical ? "Vertical COMPRESSED" : "Horizontal COMPRESSED", 3, "images\\compressed", ref width, ref height);
                                break;

                            case JpegType.Thumbnail:
                                isOK = ProcessJpeg(base_document, vertical ? "Vertical THUMBNAIL" : "Horizontal THUMBNAIL", 3, "pages\\thumbnail", ref width, ref height);
                                break;

                            default:
                                Debug.Assert(false, "OutputJpeg called with unknown jpeg type");
                                break;
                        }
                    }
                    else
                    {
                        s_error_code = ErrorCode.IllegalCall;   // not sure what happened - have source loaded, but no active document? 
                    }
                }
                else
                    s_error_code = ErrorCode.NoDiskRootSet;
            }
            else
                s_error_code = ErrorCode.NoActiveSourceDoc;
        }
        else
            s_error_code = ErrorCode.PhotoshopNotFound;

        return isOK;
    }



    /*
     * =======================================================================================
     * UnloadSourceFile
     * 
     * PUBLIC 
     * 
     * API to call when done processing a source file. After this, the user can call either 
     * LoadAndPrepareSourceFile for the next source file, or QuitApp
     * 
     */
    public static bool UnloadSourceFile()
    {
        bool isOK = false;

        if (s_app != null)
        {
            if (s_source_loaded)
            {
                isOK = true;
                s_source_loaded = false;
                s_app.ActiveDocument.Close(Photoshop.PsSaveOptions.psDoNotSaveChanges);
            }
            else
                s_error_code = ErrorCode.NoActiveSourceDoc;
        }
        else
            s_error_code = ErrorCode.PhotoshopNotFound;

        return isOK;
    }



    /*
     * =======================================================================================
     * QuitApp
     * 
     * PUBLIC 
     * 
     * Optional API the user can call to unload the photoshop app
     * 
     */
    public static bool QuitApp()
    {
        bool isOK = true;

        if (s_app != null)
        {
            s_app.Quit();
            s_app = null;
        }

        return isOK;
    }



    /*
     * =======================================================================================
     * GetErrorCode
     * 
     * PUBLIC 
     * 
     * After an API fails, this may be called to get an extended error code
     * 
     */
    public static int GetErrorCode()
    {
        return (int)s_error_code;
    }
}
