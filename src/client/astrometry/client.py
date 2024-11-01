#!/usr/bin/env python3

# This file is part of the Astrometry.net suite.
# Copyright 2009 Dustin Lang
# Licensed under a 3-clause BSD style license - see LICENSE
# https://github.com/dstndstn/astrometry.net

import os
import sys
import time
import base64
import shutil
import random
import json
import argparse
from urllib.parse import urlencode, quote
from urllib.request import urlopen, Request
from urllib.error import HTTPError, URLError
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart
from email.mime.application import MIMEApplication
from email.encoders import encode_noop
from loguru import logger
import numpy as np
from astropy.io import fits


class MalformedResponse(Exception):
    pass


class RequestError(Exception):
    pass


class Tan:
    def __init__(self, wcsfn, wcsext=0):
        with fits.open(wcsfn) as hdul:
            header = hdul[wcsext].header
            self.crval = [header['CRVAL1'], header['CRVAL2']]
            self.crpix = [header['CRPIX1'], header['CRPIX2']]
            self.cd = [
                [header['CD1_1'], header['CD1_2']],
                [header['CD2_1'], header['CD2_2']]
            ]
            self.imagew = header['NAXIS1']
            self.imageh = header['NAXIS2']

    def world_to_pixel(self, ra, dec):
        # Convert world coordinates to pixel coordinates
        ra_rad = np.deg2rad(ra)
        dec_rad = np.deg2rad(dec)
        crval_rad = np.deg2rad(self.crval)

        delta_ra = ra_rad - crval_rad[0]
        delta_dec = dec_rad - crval_rad[1]

        x = self.crpix[0] + delta_ra / self.cd[0][0]
        y = self.crpix[1] + delta_dec / self.cd[1][1]

        return x, y

    def pixel_to_world(self, x, y):
        # Convert pixel coordinates to world coordinates
        delta_x = x - self.crpix[0]
        delta_y = y - self.crpix[1]

        ra_rad = np.rad2deg(self.crval[0] + delta_x * self.cd[0][0])
        dec_rad = np.rad2deg(self.crval[1] + delta_y * self.cd[1][1])

        return ra_rad, dec_rad


class Client:
    default_url = 'https://nova.astrometry.net/api/'

    def __init__(self, apiurl=default_url):
        self.session = None
        self.apiurl = apiurl

    def get_url(self, service):
        return f"{self.apiurl}{service}"

    def send_request(self, service, args=None, file_args=None):
        if args is None:
            args = {}
        if self.session is not None:
            args.update({'session': self.session})
        logger.debug(f"Request Arguments: {args}")

        json_data = json.dumps(args)
        logger.debug(f"Sending JSON: {json_data}")
        url = self.get_url(service)
        logger.debug(f"Sending to URL: {url}")

        headers = {}
        data = b""

        if file_args:
            boundary_key = ''.join(random.choices('0123456789', k=19))
            boundary = f'==============={boundary_key}=='
            headers['Content-Type'] = f'multipart/form-data; boundary="{boundary}"'

            data_pre = (
                f'--{boundary}\r\n'
                'Content-Type: text/plain\r\n'
                'MIME-Version: 1.0\r\n'
                'Content-disposition: form-data; name="request-json"\r\n'
                '\r\n'
                f'{json_data}\r\n'
                f'--{boundary}\r\n'
                'Content-Type: application/octet-stream\r\n'
                'MIME-Version: 1.0\r\n'
                f'Content-disposition: form-data; name="file"; filename="{file_args[0]}"\r\n'
                '\r\n'
            ).encode('utf-8')
            data_post = f'\r\n--{boundary}--\r\n'.encode('utf-8')
            data = data_pre + file_args[1] + data_post
        else:
            headers['Content-Type'] = 'application/x-www-form-urlencoded'
            data = urlencode({'request-json': json_data}).encode('utf-8')

        try:
            request = Request(url=url, headers=headers, data=data)
            with urlopen(request) as response:
                status_code = response.status
                logger.debug(f"Received HTTP status code: {status_code}")
                response_text = response.read().decode('utf-8')
                logger.debug(f"Received response: {response_text}")
                result = json.loads(response_text)
                if result.get('status') == 'error':
                    err_msg = result.get('errormessage', '(none)')
                    raise RequestError(f'Server error message: {err_msg}')
                return result
        except (HTTPError, URLError) as e:
            logger.error(f"HTTP Error: {e}")
            raise RequestError(f"HTTP Error occurred: {e}")
        except Exception as e:
            logger.exception("An unexpected error occurred.")
            raise

    def login(self, apikey):
        args = {'apikey': apikey}
        result = self.send_request('login', args)
        sess = result.get('session')
        if not sess:
            raise RequestError('No session in result')
        self.session = sess
        logger.debug(f"Logged in with session: {self.session}")

    def _get_upload_args(self, **kwargs):
        args = {}
        params = [
            ('allow_commercial_use', 'd', str),
            ('allow_modifications', 'd', str),
            ('publicly_visible', 'y', str),
            ('scale_units', None, str),
            ('scale_type', None, str),
            ('scale_lower', None, float),
            ('scale_upper', None, float),
            ('scale_est', None, float),
            ('scale_err', None, float),
            ('center_ra', None, float),
            ('center_dec', None, float),
            ('parity', None, int),
            ('radius', None, float),
            ('downsample_factor', None, int),
            ('positional_error', None, float),
            ('tweak_order', None, int),
            ('crpix_center', None, bool),
            ('invert', None, bool),
            ('image_width', None, int),
            ('image_height', None, int),
            ('x', None, list),
            ('y', None, list),
            ('album', None, str),
        ]
        for key, default, typ in params:
            value = kwargs.get(key, default)
            if value is not None:
                try:
                    args[key] = typ(value)
                except ValueError as e:
                    logger.error(f"Invalid type for {key}: {e}")
        logger.debug(f"Upload arguments: {args}")
        return args

    def url_upload(self, url, **kwargs):
        args = {'url': url}
        args.update(self._get_upload_args(**kwargs))
        result = self.send_request('url_upload', args)
        return result

    def upload(self, fn=None, **kwargs):
        args = self._get_upload_args(**kwargs)
        file_args = None
        if fn:
            if not os.path.exists(fn):
                logger.error(f"File {fn} does not exist.")
                raise FileNotFoundError(f"File {fn} does not exist.")
            with open(fn, 'rb') as f:
                file_args = (os.path.basename(fn), f.read())
        return self.send_request('upload', args, file_args)

    def submission_images(self, subid):
        result = self.send_request('submission_images', {'subid': subid})
        return result.get('image_ids', [])

    def overlay_plot(self, service, outfn, wcsfn, wcsext=0):
        try:
            wcs = Tan(wcsfn, wcsext)
            params = {
                'crval1': wcs.crval[0],
                'crval2': wcs.crval[1],
                'crpix1': wcs.crpix[0],
                'crpix2': wcs.crpix[1],
                'cd11': wcs.cd[0],
                'cd12': wcs.cd[1],
                'cd21': wcs.cd[2],
                'cd22': wcs.cd[3],
                'imagew': wcs.imagew,
                'imageh': wcs.imageh
            }
            result = self.send_request(service, {'wcs': params})
            if result.get('status') == 'success':
                plot_data = base64.b64decode(result['plot'])
                with open(outfn, 'wb') as f:
                    f.write(plot_data)
                logger.debug(f"Overlay plot saved to {outfn}")
            else:
                logger.error(
                    f"Overlay plot failed with status: {result.get('status')}")
        except Exception as e:
            logger.exception("Failed to create overlay plot.")
            raise

    def sdss_plot(self, outfn, wcsfn, wcsext=0):
        self.overlay_plot('sdss_image_for_wcs', outfn, wcsfn, wcsext)

    def galex_plot(self, outfn, wcsfn, wcsext=0):
        self.overlay_plot('galex_image_for_wcs', outfn, wcsfn, wcsext)

    def myjobs(self):
        result = self.send_request('myjobs')
        return result.get('jobs', [])

    def job_status(self, job_id, justdict=False):
        result = self.send_request(f'jobs/{job_id}')
        if justdict:
            return result
        else:
            status = result.get('status')
            logger.debug(f"Job {job_id} status: {status}")
            if status == 'success':
                logger.debug(f"Job {job_id} completed successfully.")
            elif status == 'failure':
                logger.debug(f"Job {job_id} failed.")
            else:
                logger.debug(f"Job {job_id} status: {status}")
            return status

    def annotate_data(self, job_id):
        result = self.send_request(f'jobs/{job_id}/annotations')
        return result

    def sub_status(self, sub_id, justdict=False):
        result = self.send_request(f'submissions/{sub_id}')
        if justdict:
            return result
        else:
            return result.get('status')

    def jobs_by_tag(self, tag, exact=False):
        exact_option = 'exact=yes' if exact else ''
        result = self.send_request(
            f'jobs_by_tag?query={quote(tag.strip())}&{exact_option}')
        return result

    def delete_job(self, job_id):
        try:
            result = self.send_request(f'jobs/{job_id}/delete')
            return result
        except RequestError as e:
            logger.error(f"Failed to delete job {job_id}: {e}")
            return None


def main():
    logger.debug(f"Run arguments: {sys.argv}")
    parser = argparse.ArgumentParser()
    parser.add_argument('--server', default=Client.default_url,
                        help='Set the base server URL')
    parser.add_argument(
        '--apikey', '-k', help='Astrometry.net API key; if not specified, will check environment variable AN_API_KEY')
    parser.add_argument('--upload', '-u', help='Upload file')
    parser.add_argument(
        '--upload-xy', help='Upload FITS x,y table in JSON format')
    parser.add_argument('--wait', '-w', action='store_true',
                        help='Monitor job status after submission')
    parser.add_argument(
        '--wcs', help='Download generated wcs.fits file and save to specified filename; defaults to --wait if using --urlupload or --upload')
    parser.add_argument(
        '--newfits', help='Download generated new image fits file and save to specified filename; defaults to --wait if using --urlupload or --upload')
    parser.add_argument(
        '--corr', help='Download generated corr.fits file and save to specified filename; defaults to --wait if using --urlupload or --upload')
    parser.add_argument(
        '--kmz', help='Download generated kmz file and save to specified filename; defaults to --wait if using --urlupload or --upload')
    parser.add_argument(
        '--annotate', '-a', help='Store annotation information in specified file in JSON format; defaults to --wait if using --urlupload or --upload')
    parser.add_argument('--urlupload', '-U',
                        help='Upload file from specified URL')
    parser.add_argument('--scale-units', choices=[
                        'arcsecperpix', 'arcminwidth', 'degwidth', 'focalmm'], help='Units for scale estimation')
    parser.add_argument('--scale-lower', type=float,
                        help='Lower bound for scale')
    parser.add_argument('--scale-upper', type=float,
                        help='Upper bound for scale')
    parser.add_argument('--scale-est', type=float, help='Scale estimate')
    parser.add_argument('--scale-err', type=float,
                        help='Scale estimate error (percentage)')
    parser.add_argument('--ra', dest='center_ra', type=float, help='RA center')
    parser.add_argument('--dec', dest='center_dec',
                        type=float, help='Dec center')
    parser.add_argument('--radius', type=float,
                        help='Search radius around RA, Dec center')
    parser.add_argument('--downsample', type=int,
                        help='Downsample image by this factor')
    parser.add_argument('--positional_error', type=float,
                        help='Number of pixels a star may deviate from its true position')
    parser.add_argument(
        '--parity', choices=['0', '1'], help='Parity of the image (flipping)')
    parser.add_argument('--tweak-order', type=int,
                        help='SIP distortion order (default: 2)')
    parser.add_argument('--crpix-center', action='store_true',
                        help='Set reference point to image center')
    parser.add_argument('--invert', action='store_true',
                        help='Invert image before detecting sources')
    parser.add_argument('--image-width', type=int,
                        help='Set image width for x,y list')
    parser.add_argument('--image-height', type=int,
                        help='Set image height for x,y list')
    parser.add_argument('--album', type=str,
                        help='Add image to album with specified title string')
    parser.add_argument('--sdss', nargs=2, metavar=('WCSFILE',
                        'OUTFILE'), help='Plot SDSS image for given WCS file')
    parser.add_argument('--galex', nargs=2, metavar=('WCSFILE',
                        'OUTFILE'), help='Plot GALEX image for given WCS file')
    parser.add_argument('--jobid', '-i', type=int,
                        help='Retrieve results for jobId instead of submitting new image')
    parser.add_argument('--substatus', '-s', help='Get submission status')
    parser.add_argument('--jobstatus', '-j', help='Get job status')
    parser.add_argument('--jobs', '-J', action='store_true',
                        help='Get all my jobs')
    parser.add_argument('--jobsbyexacttag', '-T',
                        help='Get list of jobs with exact tag match')
    parser.add_argument('--jobsbytag', '-t',
                        help='Get list of jobs associated with given tag')
    parser.add_argument('--private', '-p', dest='public', action='store_const',
                        const='n', default='y', help='Hide this submission from other users')
    parser.add_argument('--allow_mod_sa', '-m', dest='allow_mod', action='store_const',
                        const='sa', default='d', help='Allow derivative works under the same license')
    parser.add_argument('--no_mod', '-M', dest='allow_mod', action='store_const',
                        const='n', default='d', help='Do not allow derivative works')
    parser.add_argument('--no_commercial', '-c', dest='allow_commercial',
                        action='store_const', const='n', default='d', help='Prohibit commercial use')
    opt = parser.parse_args()

    apikey = opt.apikey or os.environ.get('AN_API_KEY')
    if not apikey:
        parser.print_help()
        logger.error(
            'API key must be specified with --apikey or set the environment variable AN_API_KEY')
        sys.exit(1)

    client = Client(apiurl=opt.server)
    try:
        client.login(apikey)
    except RequestError as e:
        logger.error(f"Login failed: {e}")
        sys.exit(1)

    sub_id = None
    solved_id = opt.jobid
    wait = opt.wait

    if opt.upload or opt.urlupload or opt.upload_xy:
        if opt.wcs or opt.kmz or opt.newfits or opt.corr or opt.annotate:
            wait = True

        upload_args = {
            'allow_commercial_use': opt.allow_commercial,
            'allow_modifications': opt.allow_mod,
            'publicly_visible': opt.public
        }

        if opt.scale_lower and opt.scale_upper:
            upload_args.update(scale_lower=opt.scale_lower,
                               scale_upper=opt.scale_upper, scale_type='ul')
        elif opt.scale_est and opt.scale_err:
            upload_args.update(scale_est=opt.scale_est,
                               scale_err=opt.scale_err, scale_type='ev')
        elif opt.scale_lower or opt.scale_upper:
            upload_args.update(scale_type='ul')
            if opt.scale_lower:
                upload_args['scale_lower'] = opt.scale_lower
            if opt.scale_upper:
                upload_args['scale_upper'] = opt.scale_upper

        for key in ['scale_units', 'center_ra', 'center_dec', 'radius', 'downsample', 'positional_error',
                    'tweak_order', 'crpix_center', 'invert', 'image_width', 'image_height', 'album']:
            value = getattr(opt, key, None)
            if value is not None:
                upload_args[key] = value
        if opt.parity is not None:
            upload_args['parity'] = int(opt.parity)

        if opt.upload:
            upload_result = client.upload(opt.upload, **upload_args)
        elif opt.upload_xy:
            try:
                from astrometry.util.fits import fits_table
                T = fits_table(opt.upload_xy)
                upload_args.update(x=[float(x)
                                   for x in T.x], y=[float(y) for y in T.y])
                upload_result = client.upload(**upload_args)
            except ImportError as e:
                logger.error("Cannot import fits_table to upload xy data.")
                sys.exit(1)
        elif opt.urlupload:
            upload_result = client.url_upload(opt.urlupload, **upload_args)
        else:
            logger.error("No upload source specified.")
            sys.exit(1)

        if upload_result['status'] != 'success':
            logger.error(f"Upload failed: {upload_result.get('status')}")
            sys.exit(1)

        sub_id = upload_result['subid']

    if wait and not solved_id:
        if not sub_id:
            logger.error("Cannot wait without submission ID or job ID!")
            sys.exit(1)
        while True:
            status_result = client.sub_status(sub_id, justdict=True)
            jobs = status_result.get('jobs', [])
            if any(jobs):
                solved_id = next((j for j in jobs if j is not None), None)
                if solved_id is not None:
                    logger.debug(f"Found job ID: {solved_id}")
                    break
            time.sleep(5)
        while True:
            job_result = client.job_status(solved_id, justdict=True)
            status = job_result.get('status')
            if status == 'success':
                break
            elif status == 'failure':
                logger.error("Image analysis failed.")
                sys.exit(1)
            time.sleep(5)

    if solved_id:
        retrieve_items = []
        base_url = opt.server.replace('/api/', '')
        if opt.wcs:
            url = f"{base_url}/wcs_file/{solved_id}"
            retrieve_items.append((url, opt.wcs))
        if opt.kmz:
            url = f"{base_url}/kml_file/{solved_id}/"
            retrieve_items.append((url, opt.kmz))
        if opt.newfits:
            url = f"{base_url}/new_fits_file/{solved_id}/"
            retrieve_items.append((url, opt.newfits))
        if opt.corr:
            url = f"{base_url}/corr_file/{solved_id}"
            retrieve_items.append((url, opt.corr))

        for url, filename in retrieve_items:
            logger.debug(f"Downloading file from {url} to {filename}")
            try:
                with urlopen(url) as response, open(filename, 'wb') as out_file:
                    shutil.copyfileobj(response, out_file)
                logger.debug(f"Saved to {filename}")
            except Exception as e:
                logger.error(f"Cannot download file from {url}: {e}")
                sys.exit(1)

        if opt.annotate:
            annotation_data = client.annotate_data(solved_id)
            with open(opt.annotate, 'w') as f:
                json.dump(annotation_data, f)
            logger.debug(f"Annotation data saved to {opt.annotate}")

    if opt.sdss:
        wcsfn, outfn = opt.sdss
        client.sdss_plot(outfn, wcsfn)
    if opt.galex:
        wcsfn, outfn = opt.galex
        client.galex_plot(outfn, wcsfn)

    if opt.substatus:
        status = client.sub_status(opt.substatus)
        logger.debug(f"Submission {opt.substatus} status: {status}")
    if opt.jobstatus:
        status = client.job_status(opt.jobstatus)
        logger.debug(f"Job {opt.jobstatus} status: {status}")

    if opt.jobsbytag:
        jobs = client.jobs_by_tag(opt.jobsbytag, exact=False)
        logger.debug(f"Jobs with tag {opt.jobsbytag}: {jobs}")
    if opt.jobsbyexacttag:
        jobs = client.jobs_by_tag(opt.jobsbyexacttag, exact=True)
        logger.debug(f"Jobs with exact tag {opt.jobsbyexacttag}: {jobs}")

    if opt.jobs:
        jobs = client.myjobs()
        logger.debug(f"My jobs: {jobs}")

    # Additional functionality: Delete a job
    if opt.delete_job:
        delete_result = client.delete_job(opt.delete_job)
        if delete_result and delete_result.get('status') == 'success':
            logger.debug(f"Job {opt.delete_job} deleted successfully.")
        else:
            logger.error(f"Failed to delete job {opt.delete_job}.")


if __name__ == '__main__':
    main()
