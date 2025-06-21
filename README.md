# Windows Hosts File Editor

A small utility to edit the Windows `hosts` file easily.

I created this tool because I needed to map custom domain names to IP addresses when working with Kubernetes locally. Rather than edit the file manually every time, I decided to build a simple graphical tool to do the job.

Watch in action : https://www.youtube.com/watch?v=N-e_tncrUwk
App icon from https://github.com/legacy-icons/famfamfam-silk

## Features

- View and search entries in the Windows `hosts` file
- Add new domain–IP mappings
- Delete or edit existing entries
- Save changes (admin rights required)
- Clean and minimal Qt-based UI

## Why?

When working with local environments like Kubernetes, it's common to route domains (like `my-service.local`) to specific local IPs. Modifying the `hosts` file manually is error-prone and tedious — this tool makes it easier.

## Requirements

- Windows
- Qt 6 (or Qt 5 if adapted)
- Administrator privileges to save changes

## Usage

1. Run the application
2. View, add, edit, or delete entries
3. Click "Save" — the app will request elevated permissions if needed

## Notes

- Saving the hosts file requires admin rights. The app will attempt to elevate automatically when needed.
- Backup your original `hosts` file before making changes, just in case.

## License

MIT
