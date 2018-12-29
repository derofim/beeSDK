package com.sohu.tv.audiochat;

import java.io.InputStream;

public final class ZipUtil {
    private ZipUtil(){
    }

    public static java.util.List<java.io.File> getFileList(String zipFileString, boolean bContainFolder,
                                                           boolean bContainFile)throws Exception {
        java.util.List<java.io.File> fileList = new java.util.ArrayList<java.io.File>();
        java.util.zip.ZipInputStream inZip =
                new java.util.zip.ZipInputStream(new java.io.FileInputStream(zipFileString));
        java.util.zip.ZipEntry zipEntry;
        String szName = "";
        while ((zipEntry = inZip.getNextEntry()) != null) {
            szName = zipEntry.getName();
            if (zipEntry.isDirectory()) {
                // get the folder name of the widget
                szName = szName.substring(0, szName.length() - 1);
                java.io.File folder = new java.io.File(szName);
                if (bContainFolder) {
                    fileList.add(folder);
                }
            } else {
                java.io.File file = new java.io.File(szName);
                if (bContainFile) {
                    fileList.add(file);
                }
            }
        }//end of while
        inZip.close();
        return fileList;
    }

    public static InputStream upZip(String zipFilePath, String fileString)throws Exception {
        java.util.zip.ZipFile zipFile = new java.util.zip.ZipFile(zipFilePath);
        java.util.zip.ZipEntry zipEntry = zipFile.getEntry(fileString);

        return zipFile.getInputStream(zipEntry);
    }

    public static void unZipFolder(InputStream input, String outPathString)throws Exception {
        java.util.zip.ZipInputStream inZip = new java.util.zip.ZipInputStream(input);
        java.util.zip.ZipEntry zipEntry = null;
        String szName = "";

        while ((zipEntry = inZip.getNextEntry()) != null) {
            szName = zipEntry.getName();

            if (zipEntry.isDirectory()) {
                // get the folder name of the widget
                szName = szName.substring(0, szName.length() - 1);
                java.io.File folder = new java.io.File(outPathString + java.io.File.separator + szName);
                folder.mkdirs();
            } else {
                java.io.File file = new java.io.File(outPathString + java.io.File.separator + szName);
                file.createNewFile();
                // get the output stream of the file
                java.io.FileOutputStream out = new java.io.FileOutputStream(file);
                int len;
                byte[] buffer = new byte[1024];
                // read (len) bytes into buffer
                while ((len = inZip.read(buffer)) != -1) {
                    // write (len) byte from buffer at the position 0
                    out.write(buffer, 0, len);
                    out.flush();
                }
                out.close();
            }
        }//end of while
        inZip.close();
    }

    public static void unZipFolder(String zipFileString, String outPathString)throws Exception {
        unZipFolder(new java.io.FileInputStream(zipFileString),outPathString);
    }//end of func

    public static void zipFolder(String srcFilePath, String zipFilePath)throws Exception {
        java.util.zip.ZipOutputStream outZip =
                new java.util.zip.ZipOutputStream(new java.io.FileOutputStream(zipFilePath));

        java.io.File file = new java.io.File(srcFilePath);

        zipFiles(file.getParent()+java.io.File.separator, file.getName(), outZip);

        outZip.finish();
        outZip.close();
    }//end of func

    private static void zipFiles(String folderPath, String filePath,
                                 java.util.zip.ZipOutputStream zipOut)throws Exception{
        if(zipOut == null){
            return;
        }

        java.io.File file = new java.io.File(folderPath+filePath);

        if (file.isFile()) {
            java.util.zip.ZipEntry zipEntry =  new java.util.zip.ZipEntry(filePath);
            java.io.FileInputStream inputStream = new java.io.FileInputStream(file);
            zipOut.putNextEntry(zipEntry);

            int len;
            byte[] buffer = new byte[4096];

            while((len=inputStream.read(buffer)) != -1) {
                zipOut.write(buffer, 0, len);
            }

            zipOut.closeEntry();
        } else {
            String fileList[] = file.list();

            if (fileList.length <= 0) {
                java.util.zip.ZipEntry zipEntry =
                        new java.util.zip.ZipEntry(filePath+java.io.File.separator);
                zipOut.putNextEntry(zipEntry);
                zipOut.closeEntry();
            }

            for (int i = 0; i < fileList.length; i++) {
                zipFiles(folderPath, filePath+java.io.File.separator+fileList[i], zipOut);
            }//end of for

        }//end of if

    }//end of func
}