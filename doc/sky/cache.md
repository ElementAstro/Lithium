# `CacheSkySurvey`

## Overview

The `CacheSkySurvey` class is responsible for managing a cache of sky survey images. It handles operations such as initialization, saving images to the cache, retrieving images from the cache, and clearing the cache. The class is designed to work with image data and metadata to ensure efficient retrieval and storage of sky survey images.

### Overall Flowchart

```mermaid
graph TD
    A[Start] --> B{Cache Exists?}
    B -->|Yes| C[Load Cache]
    B -->|No| D[Create Cache]
    C --> E[Ensure Backwards Compatibility]
    D --> E
    E --> F[Initialization Complete]
    F --> G[Cache Operations: Save, Retrieve, Clear]
    G --> H[End]
```

---

## Step-by-Step Flowcharts

### 1. Initialization

The `Initialize` method is called when a `CacheSkySurvey` object is instantiated. It checks if the cache directory and `CacheInfo.xml` file exist. If not, it creates them. The method also ensures backward compatibility with previous versions by adding missing attributes.

#### Initialization Flowchart

```mermaid
graph TD
    A[Start Initialization] --> B{Cache Directory Exists?}
    B -->|Yes| C{CacheInfo.xml Exists?}
    C -->|Yes| D[Load CacheInfo.xml]
    D --> E[Ensure Backwards Compatibility]
    C -->|No| F[Create CacheInfo.xml]
    F --> G[Save Empty Cache to File]
    B -->|No| H[Create Cache Directory]
    H --> F
    E --> I[Initialization Complete]
    G --> I
    I --> J[Ready for Cache Operations]
```

### 2. Saving an Image to Cache

The `SaveImageToCache` method saves a `SkySurveyImage` to the cache. It first checks if the image already exists in the cache. If not, it generates file paths, saves the image, creates thumbnails, and updates the cache file.

#### Save Image Flowchart

```mermaid
graph TD
    A[Start Save Image] --> B[Check If Image Exists in Cache]
    B -->|Exists| C[Return Existing Image]
    B -->|Not Exists| D[Generate File Paths]
    D --> E[Save Image to Disk]
    E --> F[Create Thumbnails]
    F --> G[Update Cache XML]
    G --> H[Save Cache XML to File]
    H --> I[Return New Image]
```

### 3. Retrieving an Image from Cache

The `GetImage` method retrieves a sky survey image from the cache based on various parameters like source, RA, Dec, rotation, and FOV. It searches the cache for a matching image and loads it.

#### Retrieve Image Flowchart

```mermaid
graph TD
    A[Start Retrieve Image] --> B[Search for Image in Cache]
    B -->|Found| C[Load Image from Disk]
    C --> D[Return Image]
    B -->|Not Found| E[Return Null]
```

### 4. Clearing the Cache

The `Clear` method deletes all files and directories in the cache path and reinitializes the cache.

#### Clear Cache Flowchart

```mermaid
graph TD
    A[Start Clear Cache] --> B[Delete All Files in Cache Path]
    B --> C[Delete All Directories in Cache Path]
    C --> D[Reinitialize Cache]
    D --> E[Cache Cleared]
```

### 5. Deleting an Image from Cache

The `DeleteFromCache` method removes a specific image from the cache by deleting the image file and its associated thumbnails.

#### Delete Image Flowchart

```mermaid
graph TD
    A[Start Delete Image] --> B[Check if Element is Valid]
    B -->|Valid| C[Get Image and Thumbnail Paths]
    C --> D[Delete Image and Thumbnails]
    D --> E[Remove Element from Cache]
    E --> F[Save Updated Cache to File]
    F --> G[Image Deleted]
    B -->|Invalid| H[Abort Deletion]
```
